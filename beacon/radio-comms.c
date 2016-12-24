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
#include "non-volatile.h"
#include "robot.h"

// event triggered when new robot locations are available
EVENTSOURCE_DECL(radioEvent);

// RX/TX buffer
#define RADIO_BUF_LEN 100
static uint8_t radioBuffer[RADIO_BUF_LEN];

// timestamp of the last start-of-frame
static int64_t sofTS = -1;
static systime_t sofSystime = -1;

// connected robots IDs
static uint8_t robotIDs[MAX_CONNECTED_ROBOTS];

// for slave beacon : whether beacon is known by master beacon
// for master beacon : bit0: slave beacon 1 connected, bit1: slave beacon 2 connected
static int sbConnected = 0;
// master beacon specific : session ID, randomly generated ID at each startup of the MB
static uint8_t sessionID;

// adaped from a code by Samuel Tardieu (see rfc1149.net)
static void sleepUntil(systime_t previous, systime_t period) {
	systime_t future = previous + period;

	chSysLock();
	systime_t now = chVTGetSystemTimeX();

	int mustDelay = now < previous ?
		(now < future && future < previous) :
		(now < future || future < previous);
	if (mustDelay)
		chThdSleepS(future - now);
	chSysUnlock();
}

static void parseSOF(int sofLength) {
	int i;

	// get start-of-frame reception time
	sofTS = getRXtimestamp();
	sofSystime = chVTGetSystemTime();

	// update connected flag
	sbConnected = ((radioBuffer[3] & 0x01) && (deviceUID == 253))
				|| ((radioBuffer[3] & 0x02) && (deviceUID == 254));

	// retrieve active robots list, for ranging
	for(i=4; i < sofLength; i++) {
		robotIDs[i-4] = radioBuffer[i];
	}
	robotIDs[sofLength - 4] = 0;
}

static void answerBeaconRead(void) {
	int i = 0, msg_index = 2;
	radioBuffer[0] = BEACON_READ_MSG_ID;
	radioBuffer[1] = 0;

	while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0 && robotIDs[i] <= MAX_CONNECTED_ROBOTS) {
		radioBuffer[msg_index++] = robots[robotIDs[i] - 1].mbDist;
		radioBuffer[msg_index++] = robots[robotIDs[i] - 1].mbDist >> 8;
		i++;
	}

	decaSend(msg_index, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
}

static void synchronizeSlaveBeacon(void) {
	int ret;

	// listen to master beacon
	switchToChannel(MB_CHANNEL);
	dwt_setrxtimeout(SYNC_RX_TIMEOUT);

	// clear distances as they will be outdated
	for(ret = 0; ret < MAX_CONNECTED_ROBOTS; ret++)
		robots[ret].mbDist = 0;

	// not synchronized : turn on red LED and turn off green LED
	palSetPad(GPIOF, GPIOF_STAT3);
	palClearPad(GPIOF, GPIOF_CAM_PWR);

	sofTS = -1;
	while(sofTS == -1) {
		if((ret = decaReceive(RADIO_BUF_LEN, radioBuffer, DWT_START_RX_IMMEDIATE)) < 0) {
			// no valid message received
			sofTS = -1;
			// allow module to cool down ten times longer
			chThdSleepMilliseconds(SYNC_RX_TIMEOUT/100);
			continue;
		}

		// message is a start of frame
		if(radioBuffer[0] == SOF_MSG_ID && radioBuffer[1] == 0xFF)
			parseSOF(ret);
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
	radioBuffer[0] = RANGING_MSG_ID;
	radioBuffer[1] = robotUID;
	decaSend(2 + dataLength, radioBuffer, 1, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);

	// receive response
	if((ret = decaReceive(RADIO_BUF_LEN, radioBuffer, NO_RX_ENABLE)) < 0)
		return 0;

	// check frame is actually our response
	if (radioBuffer[0] == RANGING_MSG_ID && radioBuffer[1] == 0) {
		double distance;
		int32_t tx_ts, rx_ts, beacon_rx_ts, beacon_hold_time;
		int distanceInCm;

		// Retrieve poll transmission and response reception timestamps
		tx_ts = dwt_readtxtimestamplo32();
		rx_ts = dwt_readrxtimestamplo32();

		// retrieve beacon RX timestamp
		beacon_rx_ts = (int) radioBuffer[2] + ((int) radioBuffer[3] << 8);
		// compute precise time between beacon poll RX and response TX
		beacon_hold_time = POLL_TO_RESP_DLY + TX_ANT_DLY - (beacon_rx_ts & 0x1FF);

		// compute distance
		distance = (rx_ts - tx_ts - beacon_hold_time) * 100 / 2.0 * DWT_TIME_UNITS * SPEED_OF_LIGHT;
		distanceInCm = distance + 200; // avoid negative values caused by wrong calibration of antenna delay

		// if distance is still negative or 0, set it to 1 for debug
		if(distanceInCm <= 0)
			distanceInCm = 1;
		return distanceInCm;
	}
	return 0;
}

static int slaveBeaconRead(int msgID, int addr, uint64_t timeInFrame) {
	int ret;

	sleepUntil(sofSystime, (timeInFrame - 1)*10);
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);

	if((ret = decaReceive(RADIO_BUF_LEN, radioBuffer, DWT_START_RX_DELAYED)) > 0) {
		if(radioBuffer[0] == msgID && radioBuffer[1] == addr)
			return ret;
	}

	// synchronisation is lost
	sofTS = -1;
	return -1;
}

static void slaveBeaconTask(void) {
	int ret;
	int beaconReadTime = deviceUID == 253 ? TIMESLOT_LENGTH : TIMESLOT_LENGTH*2;

	dwt_setrxaftertxdelay(POLL_TO_RESP_RX);

	while(1) {
		// if last start-of-frame time isn't known or beacon isn't registered
		if(sofTS == -1 || sbConnected == 0)
			synchronizeSlaveBeacon();
		else {
			if((ret = slaveBeaconRead(SOF_MSG_ID, 0xFF, FRAME_LENGTH)) > 0)
				parseSOF(ret);

			// if something went wrong, restart synchronisation
			if(sbConnected == 0 || ret < 0) {
				chThdSleepMilliseconds(FRAME_LENGTH - 4);
				continue;
			}
		}

		// wait for beacon read
		if((ret = slaveBeaconRead(BEACON_READ_MSG_ID, deviceUID, beaconReadTime)) > 0) {
			answerBeaconRead();
		} else { // no valid message received
			chThdSleepMilliseconds(FRAME_LENGTH - 4);
			continue;
		}

		// start ranging if any robot is registered
		if(robotIDs[0] != 0) {
			int i = 0;
			int timeInFrame = 3*TIMESLOT_LENGTH + 2*beaconReadTime;

			switchToChannel(deviceUID == 253 ? SB1_CHANNEL : SB2_CHANNEL);
			while(robotIDs[i] != 0 && robotIDs[i] <= MAX_CONNECTED_ROBOTS) {
				dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);
				robots[robotIDs[i] - 1].mbDist = rangeRobot(robotIDs[i], 0);

				timeInFrame += TIMESLOT_LENGTH;
				i++;
			}
			switchToChannel(MB_CHANNEL);
		}
	}
}

static int masterBeaconSend(int timeInFrame, int expectAnswer, int size) {
	sleepUntil(sofSystime, (timeInFrame - 1)*10);
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);

	// make sure TX done bit is cleared
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

	// send message and receive answer if expected
	if(expectAnswer) {
		if(decaSend(size, radioBuffer, 1, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED) < 0)
			return -4;
		chprintf(USBserial, "%d\n", (getTXtimestamp() - sofTS)/MS_TO_DWT);
		return decaReceive(RADIO_BUF_LEN, radioBuffer, NO_RX_ENABLE);
	} else if(decaSend(size, radioBuffer, 1, DWT_START_TX_DELAYED) < 0)
		return -4;

	return 0;
}

static void sendSOF(void) {
	int i = 0;
	radioBuffer[0] = SOF_MSG_ID;
	radioBuffer[1] = 0xFF;
	radioBuffer[2] = sessionID;
	radioBuffer[3] = sbConnected;

	while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
		radioBuffer[i + 4] = robotIDs[i];
		i++;
	}

	if(sofTS == -1) // on first frame, send start-of-frame immediately
		decaSend(i + 4, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
	else // send start-of-frame precisely FRAME_LENGTH ms after last start-of-frame
		masterBeaconSend(FRAME_LENGTH, 0, i + 4);

	sofTS = getTXtimestamp();
	sofSystime = chVTGetSystemTime();
}

static void readSlaveBeacon(int beaconID) {
	int ret, i, j=0;
	radioBuffer[0] = BEACON_READ_MSG_ID;
	radioBuffer[1] = beaconID;

	ret = masterBeaconSend(beaconID == 253 ? TIMESLOT_LENGTH : TIMESLOT_LENGTH*2, 1, 2);
	chprintf(USBserial, "%d id = %d\n", ret, beaconID);

	if(ret > 0 && radioBuffer[0] == BEACON_READ_MSG_ID && radioBuffer[1] == 0) {
		// read and store distances
		for(i=3; i<ret; i+=2) {
			if(beaconID == 253) {
				robots[robotIDs[j] - 1].sb1Dist = radioBuffer[i];
				robots[robotIDs[j] - 1].sb1Dist &= radioBuffer[i + 1] << 8;
				robots[robotIDs[j] - 1].sb1Dist += robots[robotIDs[j] - 1].offsets->sb1;
			} else {
				robots[robotIDs[j] - 1].sb2Dist = radioBuffer[i];
				robots[robotIDs[j] - 1].sb2Dist &= radioBuffer[i + 1] << 8;
				robots[robotIDs[j] - 1].sb2Dist += robots[robotIDs[j] - 1].offsets->sb2;
			}
			j++;
		}
		if((sbConnected & 0x01) == 0 && beaconID == 253)
			chprintf(USBserial, "SB1 connected\n");
		if((sbConnected & 0x02) == 0 && beaconID == 254)
			chprintf(USBserial, "SB2 connected\n");
		// mark beacon as connected
		sbConnected = sbConnected | (beaconID == 253 ? 0x01 : 0x02);
	} else { // without answer, mark beacon as disconnected
		if((sbConnected & 0x01) && beaconID == 253)
			chprintf(USBserial, "SB1 disconnected\n");
		if((sbConnected & 0x02) && beaconID == 254)
			chprintf(USBserial, "SB2 disconnected\n");
		sbConnected = sbConnected & (beaconID == 253 ? 0xFE : 0xFD);
	}
}

static void pollNewRobot(int nextIndex) {
	int id = 0, ret, i;
	// find an available ID
	for(i=0; i<MAX_CONNECTED_ROBOTS; i++)
		if(robotIDs[i] == id) {
			i = -1;
			id++;
			if(id == MAX_CONNECTED_ROBOTS) {
				id = 0;
				break;
			}
		}

	// send poll
	radioBuffer[0] = NEW_ROBOT_MSG_ID;
	radioBuffer[1] = 0xFF;
	radioBuffer[2] = id;
	ret = masterBeaconSend(FRAME_LENGTH - TIMESLOT_LENGTH, 1, 3);

	// process answer
	if(ret == 5 && radioBuffer[0] == NEW_ROBOT_MSG_ID && radioBuffer[1] == 0
		&& nextIndex < MAX_CONNECTED_ROBOTS && id != 0)
	{
		int uid = radioBuffer[3] + (((int) radioBuffer[4]) << 8) + (((int) radioBuffer[5]) << 16);

		robotIDs[nextIndex] = id;
		robots[id - 1].mbDist = 0; // distance is not available yet
		robots[id - 1].sb1Dist = 0;
		robots[id - 1].sb2Dist = 0;
		robots[id - 1].offsets = loadOffsets(uid);

		chprintf(USBserial, "new robot connected, id=%d\n", id);
	};
}

static void masterBeaconTask(void) {
	int i, j;
	dwt_setrxaftertxdelay(0);

	// generate a session ID
	chThdSleepMilliseconds(500);
	radioBuffer[0] = 0;
	radioBuffer[1] = 0;
	decaSend(2, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
	sessionID = (dwt_readtxtimestamplo32() >> 8) + (dwt_readtxtimestamplo32() >> 16);

	chprintf(USBserial, "sessionID = %d\n", sessionID);

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
		    chEvtBroadcastFlags(&radioEvent, EVENT_MASK(0));
		    chSysUnlock();
		} else {
			// LED : red
			palSetPad(GPIOF, GPIOF_STAT3);
			palClearPad(GPIOF, GPIOF_CAM_PWR);
		}

		// send data the robots and measuse distances
		dwt_setrxaftertxdelay(POLL_TO_RESP_RX);
		i=0;
		while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
			dwt_setdelayedtrxtime((sofTS  + (i + 3)*TIMESLOT_LENGTH*MS_TO_DWT) >> 8);
			robots[robotIDs[i] - 1].mbDist =
				rangeRobot(robotIDs[i], serializeRobotData(radioBuffer, robotIDs[i]));
			// distance = 0 means robot didn't respond or an error happened
			if(robots[robotIDs[i] - 1].mbDist == 0)
				robotIDs[i] = 0; // mark robot as disconnected
			else {
				robots[robotIDs[i] - 1].mbDist += robots[robotIDs[i] - 1].offsets->mb;
				robots[robotIDs[i] - 1].status = radioBuffer[4];
			}
			i++;
		}
		dwt_setrxaftertxdelay(0);

		// send new robot polls
		pollNewRobot(i);

		// clean up robotIDs list (remove "holes" in the list)
		i=0;
		for(j=0; j<MAX_CONNECTED_ROBOTS; j++)
			if(robotIDs[j] != 0)
				robotIDs[i++] = robotIDs[j];
		while(i<MAX_CONNECTED_ROBOTS)
			robotIDs[i++] = 0;
	}
}

static THD_WORKING_AREA(waRadio, 256);
static THD_FUNCTION(radioThread, th_data) {
	event_listener_t evt_listener;

	(void) th_data;
	chRegSetThreadName("Radio");

	// initialize decawave module
	decaInit();
	// Set response timeout
	dwt_setrxtimeout(RX_TIMEOUT);

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

	if(deviceUID == 0)
		masterBeaconTask();
	else
		slaveBeaconTask();
}

void dumpConnectedDevices(BaseSequentialStream *chp, int argc, char **argv) {
	int i;
	(void) argc;
	(void) argv;

	if(deviceUID != 0) {
		chprintf(chp, "available only on master beacon\n");
		return;
	}

	if(sbConnected & 0x01)
		chprintf(chp, "Slave beacon 1 ... connected\n");
	else
		chprintf(chp, "Slave beacon 1 ... NOT connected\n");
	if(sbConnected & 0x02)
		chprintf(chp, "Slave beacon 2 ... connected\n");
	else
		chprintf(chp, "Slave beacon 2 ... NOT connected\n");

	for(i=0; i<MAX_CONNECTED_ROBOTS; i++)
		if(robotIDs[i] != 0)
			chprintf(chp, "Robot ID=%d, UID=%d\n", robotIDs[i], robots[robotIDs[i]-1].offsets->uid);
}

void startRadio(void) {
	chThdCreateStatic(waRadio, sizeof(waRadio), NORMALPRIO+1, radioThread, NULL);
}
