#include "ch.h"
#include "chvt.h"
#include "chprintf.h"
#include "chevents.h"

#include "../shared/exti-config.h"
#include "../shared/usb-config.h"
#include "../shared/deca-functions.h"
#include "../shared/decadriver/deca_device_api.h"
#include "../shared/decadriver/deca_regs.h"
#include "../shared/radio-conf.h"
#include "robot.h"

// Events sources
EVENTSOURCE_DECL(radio_event);

// RX/TX buffer
#define RADIO_BUF_LEN 100
static uint8_t radio_buffer[RADIO_BUF_LEN];

// timestamp of the last start-of-frame
static int64_t sofTS = -1;
static systime_t sofSystime = -1;

// connected robots IDs
static uint8_t robotIDs[MAX_CONNECTED_ROBOTS];

// ########### slave beacon specific ###########
// delay between start-of-frame and  time slot
#if DEVICE_UID == 253
	#define TIMESLOT_DELAY TIMESLOT_LENGTH
#else
	#define TIMESLOT_DELAY TIMESLOT_LENGTH*2
#endif
// for slave beacon : 1 if beacon is known by master beacon
static int isRegistered = 0;

// ########### master beacon specific ###########
// flags indicating the connected slave beacons : bit0: slave beacon 1, bit1: slave beacon 2
static int sbConnected = 0;

static void parseSOF(int sofLength) {
	int i;
	// update registered flag
	isRegistered = ((radio_buffer[2] & 0x01) && (DEVICE_UID == 253))
				|| ((radio_buffer[2] & 0x02) && (DEVICE_UID == 254));

	// retrieve active robots list, for ranging
	for(i=3; i < sofLength-2; i++) {
		robotIDs[i-3] = radio_buffer[i];
	}
	robotIDs[sofLength - 2] = 0;
}
static void answerBeaconRead(void) {
	int i = 0, msg_index = 2;
	radio_buffer[0] = BEACON_READ_MSG_ID;
	radio_buffer[1] = 0;

	while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0 && robotIDs[i] <= MAX_ROBOT_ID) {
		radio_buffer[msg_index++] = robots[robotIDs[i] - 1].mbDist;
		radio_buffer[msg_index++] = robots[robotIDs[i] - 1].mbDist >> 8;
		i++;
	}

	decaSend(msg_index, radio_buffer, 1, DWT_START_TX_IMMEDIATE);
}

static void synchronizeSlaveBeacon(void) {
	int ret;
	int RXflag = DWT_START_RX_IMMEDIATE;

	// listen to master beacon
	switchToChannel(MB_CHANNEL);
	dwt_setrxtimeout(SYNC_RX_TIMEOUT);

	// clear distances as they will be outdated
	for(ret = 0; ret < MAX_ROBOT_ID; ret++)
		robots[ret].mbDist = 0;

	// not synchronized : turn on red LED and turn off green LED
	palSetPad(GPIOF, GPIOF_STAT3);
	palClearPad(GPIOF, GPIOF_CAM_PWR);

	sofTS = -1;
	while(sofTS == -1 || !isRegistered) {

		if((ret = decaReceive(RADIO_BUF_LEN, radio_buffer, RXflag)) < 0) { 	// no valid message received
			sofTS = -1;
			chThdSleepMilliseconds(SYNC_RX_TIMEOUT/100); // allow module to cool down ten times longer
			continue;
		}

		// message is a start of frame
		if(radio_buffer[0] == SOF_MSG_ID && radio_buffer[1] == 0xFF) {
			sofTS = getRXtimestamp();
			sofSystime = chVTGetSystemTime();
			parseSOF(ret);
			// wait for beacon read to register
			if(!isRegistered) {
				dwt_setdelayedtrxtime((sofTS + TIMESLOT_DELAY*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);
				RXflag = DWT_START_RX_DELAYED;
			}
			continue;
		}
		// message is a beacon read
		if(radio_buffer[0] == BEACON_READ_MSG_ID && radio_buffer[1] == DEVICE_UID) {
			// register by answering beacon read
			answerBeaconRead();

			// wait and program RX enable to receive next start-of-frame
			chThdSleepMilliseconds(FRAME_LENGTH - 10);
			if(sofTS == -1) {
				uint64_t estimatedNextSOF = getRXtimestamp() - TIMESLOT_DELAY*MS_TO_DWT + FRAME_LENGTH*MS_TO_DWT;
				// use more margin as nextSOF is just an estimate
				dwt_setdelayedtrxtime((estimatedNextSOF - AHEAD_OF_TX_MARGIN*4) >> 8);
			} else {
				dwt_setdelayedtrxtime((sofTS + FRAME_LENGTH*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);
			}

			RXflag = DWT_START_RX_DELAYED;
			continue;
		}

		RXflag = DWT_START_RX_IMMEDIATE;
	}
	dwt_setrxtimeout(RX_TIMEOUT);

	// synchronized : turn off red LED and turn on green LED
	palClearPad(GPIOF, GPIOF_STAT3);
	palSetPad(GPIOF, GPIOF_CAM_PWR);
}

static int rangeRobot(int robotUID, int dataLength) {
	int ret;

	// make sure TX done bit is cleared
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

	// Send poll, and enable reception automatically to receive the answer
	radio_buffer[0] = RANGING_MSG_ID;
	radio_buffer[1] = robotUID;
	decaSend(2 + dataLength, radio_buffer, 1, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);

	// receive response
	if((ret = decaReceive(RADIO_BUF_LEN, radio_buffer, 1)) < 0)
		return 0;

	// check frame is actually our response
	if (radio_buffer[0] == RANGING_MSG_ID && radio_buffer[1] == 0) {
		double distance;
		int32_t tx_ts, rx_ts, beacon_rx_ts, beacon_hold_time;
		int distanceInCm;

		// Retrieve poll transmission and response reception timestamps
		tx_ts = dwt_readtxtimestamplo32();
		rx_ts = dwt_readrxtimestamplo32();

		// retrieve beacon RX timestamp
		beacon_rx_ts = (int) radio_buffer[2] + ((int) radio_buffer[3] << 8) +
			((int) radio_buffer[4] << 16) + ((int) radio_buffer[5] << 24);
		// compute precise time between beacon poll RX and response TX
		beacon_hold_time = POLL_TO_RESP_DLY + TX_ANT_DLY - (beacon_rx_ts & 0x1FF);

		// compute distance
		distance = (rx_ts - tx_ts - beacon_hold_time) * 100 / 2.0 * DWT_TIME_UNITS * SPEED_OF_LIGHT;
		distanceInCm = distance + 200; // avoid negative values caused by wrong calibration of antenna delay

		// if distance is still negative, set it to 1 for debug
		if(distanceInCm <= 0)
			distanceInCm = 1;
		return distanceInCm;
	}
	return 0;
}

static int slaveBeaconRead(int msgID, int addr, uint64_t timeInFrame) {
	int ret;

	// if we have to wait for too long
	if(chVTGetSystemTime() - sofSystime < MS2ST(timeInFrame - 10))
		chThdSleepUntil(sofSystime + MS2ST(timeInFrame - 5));
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);
	if((ret = decaReceive(RADIO_BUF_LEN, radio_buffer, DWT_START_RX_DELAYED)) < 0) { 	// no valid message received
		if(radio_buffer[0] == msgID && radio_buffer[1] == addr)
			return ret;
	}

	// synchronisation is lost
	sofTS = -1;
	return -1;
}

void slaveBeaconTask(void) {
	int ret;

	while(1) {
		// if last start-of-frame time isn't know or beacon isn't registered
		if(sofTS == -1 || !isRegistered)
			synchronizeSlaveBeacon();
		else {
			if((ret = slaveBeaconRead(SOF_MSG_ID, 0xFF, FRAME_LENGTH)) > 0) {
				sofTS = getRXtimestamp();
				sofSystime = chVTGetSystemTime();
				parseSOF(ret);
				if(!isRegistered) {
					chThdSleepMilliseconds(FRAME_LENGTH - 10);
					continue;
				}
			} else { // no valid message received
				chThdSleepMilliseconds(FRAME_LENGTH - 10);
				continue;
			}
		}

		// wait for beacon read
		if((ret = slaveBeaconRead(BEACON_READ_MSG_ID, DEVICE_UID, TIMESLOT_DELAY)) > 0) {
			answerBeaconRead();
		} else { // no valid message received
			chThdSleepMilliseconds(FRAME_LENGTH - 10);
			continue;
		}

		// start ranging if any robot is registered
		if(robotIDs[0] != 0) {
			int i = 0;
			int timeInFrame = DEVICE_UID == 253 ? 5*TIMESLOT_LENGTH : 7*TIMESLOT_LENGTH;

			switchToChannel(DEVICE_UID == 253 ? SB1_CHANNEL : SB2_CHANNEL);
			while(robotIDs[i] != 0 && robotIDs[i] <= MAX_ROBOT_ID) {
				dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);
				robots[robotIDs[i] - 1].mbDist = rangeRobot(robotIDs[i], 0);
				i++;
				timeInFrame += TIMESLOT_LENGTH;
			}
			switchToChannel(MB_CHANNEL);
		}
	}
}

static int masterBeaconSend(int timeInFrame, int expectAnswer, int size) {
	// if we have to wait for too long
	if(chVTGetSystemTime() - sofSystime < MS2ST(timeInFrame - 10))
		chThdSleepUntil(sofSystime + MS2ST(timeInFrame - 5));
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);

	// make sure TX done bit is cleared
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
	// send message
	if(decaSend(size, radio_buffer, 1, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED) < 0)
		return -4;

	//receive answer if expected
	if(expectAnswer)
		return decaReceive(RADIO_BUF_LEN, radio_buffer, 1);

	return 0;
}

static void sendSOF(void) {
	int i = 0;
	radio_buffer[0] = SOF_MSG_ID;
	radio_buffer[1] = 0xFF;
	radio_buffer[2] = sbConnected;

	while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
		radio_buffer[i + 3] = robotIDs[i];
		i++;
	}

	if(sofTS == -1) // on first frame, send start-of-frame immediately
		decaSend(i, radio_buffer, 1, DWT_START_TX_IMMEDIATE);
	else // send start-of-frame precisely FRAME_LENGTH ms after last start-of-frame
		masterBeaconSend(FRAME_LENGTH, 0, i);

	sofTS = getTXtimestamp();
	sofSystime = chVTGetSystemTime();
}

static void readSlaveBeacon(int beaconID) {
	int ret, i, j=0;
	radio_buffer[0] = BEACON_READ_MSG_ID;
	radio_buffer[1] = beaconID;
	if((ret = masterBeaconSend(beaconID == 253 ? TIMESLOT_LENGTH : TIMESLOT_LENGTH*2, 1, 2) > 0)
			&& radio_buffer[0] == BEACON_READ_MSG_ID && radio_buffer[1] == 0) {
		// read and store distances
		for(i=3; i<ret; i+=2) {
			if(beaconID == 253) {
				robots[robotIDs[j] - 1].sb1Dist = radio_buffer[i];
				robots[robotIDs[j] - 1].sb1Dist &= radio_buffer[i + 1] << 8;
				robots[robotIDs[j] - 1].sb1Dist += robots[robotIDs[j] - 1].sb1Offset;
			} else {
				robots[robotIDs[j] - 1].sb2Dist = radio_buffer[i];
				robots[robotIDs[j] - 1].sb2Dist &= radio_buffer[i + 1] << 8;
				robots[robotIDs[j] - 1].sb2Dist += robots[robotIDs[j] - 1].sb2Offset;
			}
			j++;
		}
		// mark beacon as connected
		sbConnected = sbConnected | (beaconID == 253 ? 0x01 : 0x02);
	} else // without answer, mark beacon as disconnected
		sbConnected = sbConnected & (beaconID == 253 ? 0xFE : 0xFD);
}

void masterBeaconTask(void) {
	int i, j, ret;

	while(1) {
		// send start-of-frame
		sendSOF();

		// retrieve slave beacon measurements
		readSlaveBeacon(253);
		readSlaveBeacon(254);

		// compute robots locations if all the beacons are available
		if(sbConnected == 0x03) {
			trilateralizeRobots();
			// LED : green
			palClearPad(GPIOF, GPIOF_STAT3);
			palSetPad(GPIOF, GPIOF_CAM_PWR);
			// send radio event (new distances and robots locations available)
			chSysLock();
		    chEvtBroadcastFlags(&radio_event, EVENT_MASK(0));
		    chSysUnlock();
		} else {
			// LED : red
			palSetPad(GPIOF, GPIOF_STAT3);
			palClearPad(GPIOF, GPIOF_CAM_PWR);
		}

		// send data the robots and measuse distances
		i=0;
		while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
			dwt_setdelayedtrxtime((sofTS  + (i + 3)*TIMESLOT_LENGTH*MS_TO_DWT) >> 8);
			robots[robotIDs[i] - 1].mbDist =
				rangeRobot(robotIDs[i], serializeRobotData(radio_buffer, robotIDs[i]));
			// distance = 0 means robot didn't respond or an error happened
			if(robots[robotIDs[i] - 1].mbDist == 0)
				robotIDs[i] = 0; // mark robot as disconnected
			else
				robots[robotIDs[i] - 1].mbDist += robots[robotIDs[i] - 1].mbOffset;
			i++;
		}

		// send new robot polls
		radio_buffer[0] = NEW_ROBOT_MSG_ID;
		radio_buffer[1] = 0xFF;
		for(j=0; j<4; j++) {
			radio_buffer[2] = i;
			ret = masterBeaconSend(FRAME_LENGTH - (4-i)*TIMESLOT_LENGTH, 1, 3);
			if( ret == 3 && radio_buffer[0] == NEW_ROBOT_MSG_ID && radio_buffer[1] == 0
					&& radio_buffer[2] < MAX_ROBOT_ID && i < MAX_CONNECTED_ROBOTS) {
				robotIDs[i++] = radio_buffer[2];
				// distance is not available yet
				robots[robotIDs[i] - 1].mbDist = 0;
			};
		}

		// clean up robotIDs list (remove "holes" in the list)
		i=0;
		for(j=0; j<MAX_CONNECTED_ROBOTS; j++)
			if(robotIDs[j] != 0)
				robotIDs[i++] = robotIDs[j];
		while(i<MAX_CONNECTED_ROBOTS)
			robotIDs[i++] = 0;
	}
}

static THD_WORKING_AREA(waRadio, 128);
static THD_FUNCTION(radioThread, th_data) {
	event_listener_t evt_listener;

	(void) th_data;
	chRegSetThreadName("Radio");

	// initialize decawave module
	decaInit();
	// Set expected response's delay and timeout
	dwt_setrxaftertxdelay(POLL_TO_RESP_RX);
	dwt_setrxtimeout(RX_TIMEOUT);

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

	if(DEVICE_UID == 0)
		masterBeaconTask();
	else
		slaveBeaconTask();
}

void startRadio(void) {
	chThdCreateStatic(waRadio, sizeof(waRadio), NORMALPRIO+1, radioThread, NULL);
}
