#include "ch.h"
#include "chvt.h"
#include "chevents.h"
#include "chprintf.h"

#include "usbconf.h"
#include "exticonf.h"
#include "../shared/decafunctions.h"
#include "../shared/decadriver/deca_device_api.h"
#include "../shared/decadriver/deca_regs.h"
#include "../shared/radioconf.h"
#include "nonvolatile.h"
#include "robot.h"

// event triggered when new robot locations are available
EVENTSOURCE_DECL(radioEvent);
// event triggered when payload has been sent and robot's answer received
EVENTSOURCE_DECL(payloadEvent);

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
// master beacon specific :
static uint8_t sessionID; // session ID, randomly generated ID at each startup of the MB
static int restartMB = 0; // soft reset for master beacon radio task
static uint16_t date = 0;

static void parseSOF(int sofLength) {
	int i;

	// get start-of-frame reception time
	sofTS = getRXtimestamp();
	sofSystime = chVTGetSystemTime();

	// update connected flag
	sbConnected = ((radioBuffer[3] & 0x01) != 0 && deviceUID == 253)
	           || ((radioBuffer[3] & 0x02) != 0 && deviceUID == 254);

	// retrieve active robots list, for ranging
	for(i=6; i < sofLength; i++) {
		robotIDs[i-6] = radioBuffer[i];
	}
	robotIDs[sofLength - 6] = 0;
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

	// clear distances as they will be outdated
	for(ret = 0; ret < MAX_CONNECTED_ROBOTS; ret++)
		robots[ret].mbDist = 0;

	dwt_setrxtimeout(SYNC_RX_TIMEOUT);
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
}

// measure the distance from the beacon to a robot and send some data if needed
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

	sleepUntil(sofSystime, timeInFrame - 1);
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
	int ret, i, timeInFrame;
	int beaconReadTime = deviceUID == 253 ? TIMESLOT_LENGTH : TIMESLOT_LENGTH*2;

	dwt_setrxaftertxdelay(POLL_TO_RESP_RX);

	while(1) {
		// if last start-of-frame time isn't known or beacon isn't registered
		if(sofTS == -1) {
			synchronizeSlaveBeacon();
		} else {
			ret = slaveBeaconRead(SOF_MSG_ID, 0xFF, FRAME_LENGTH);
			if(ret > 0) {
				parseSOF(ret);
			} else {
				// if something went wrong, restart synchronisation
				sofTS = -1;
				chThdSleepMilliseconds(FRAME_LENGTH - 4);
				continue;
			}
		}

		// wait for beacon read
		if(slaveBeaconRead(BEACON_READ_MSG_ID, deviceUID, beaconReadTime) > 0) {
			answerBeaconRead();
		} else { // no valid message received
			chThdSleepMilliseconds(FRAME_LENGTH - TIMESLOT_LENGTH*3);
			continue;
		}

		// range all connected robots
		i = 0;
		timeInFrame = 3*TIMESLOT_LENGTH + beaconReadTime;

		while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0 && robotIDs[i] <= MAX_CONNECTED_ROBOTS) {
			dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);
			robots[robotIDs[i] - 1].mbDist = rangeRobot(robotIDs[i], 0);

			timeInFrame += 3*TIMESLOT_LENGTH;
			i++;
		}
	}
}

static int masterBeaconSend(int timeInFrame, int expectAnswer, int size) {
	sleepUntil(sofSystime, timeInFrame - 1);
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT) >> 8);

	// make sure TX done bit is cleared
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

	// send message and receive answer if expected
	if(expectAnswer) {
		if(decaSend(size, radioBuffer, 1, DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED) < 0)
			return -4;
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
	radioBuffer[4] = date;
	radioBuffer[5] = date >> 8;

	while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
		radioBuffer[i + 6] = robotIDs[i];
		i++;
	}

	if(sofTS == -1) // on first frame, send start-of-frame immediately
		decaSend(i + 6, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
	else // send start-of-frame precisely FRAME_LENGTH ms after last start-of-frame
		masterBeaconSend(FRAME_LENGTH, 0, i + 6);

	sofTS = getTXtimestamp();
	sofSystime = chVTGetSystemTime();
	chSysLock(); // make sure increment is atomic
	date++;
	chSysUnlock();
}

// retrieve distances collected by a slave beacon
static void readSlaveBeacon(int beaconID) {
	int ret, dist, i, j=0;
	radioBuffer[0] = BEACON_READ_MSG_ID;
	radioBuffer[1] = beaconID;

	ret = masterBeaconSend(beaconID == 253 ? TIMESLOT_LENGTH : TIMESLOT_LENGTH*2, 1, 2);
	if(ret > 0 && radioBuffer[0] == BEACON_READ_MSG_ID && radioBuffer[1] == 0) {
		// read and store distances
		for(i=2; i<ret; i+=2) {
			dist = radioBuffer[i] + (radioBuffer[i + 1] << 8);
			if(beaconID == 253) {
				dist = dist*robots[robotIDs[j] - 1].offsets->sb1Coeff/1000;
				robots[robotIDs[j] - 1].sb1Dist = dist + robots[robotIDs[j] - 1].offsets->sb1;
			} else {
				dist = dist*robots[robotIDs[j] - 1].offsets->sb2Coeff/1000;
				robots[robotIDs[j] - 1].sb2Dist = dist + robots[robotIDs[j] - 1].offsets->sb2;
			}
			j++;
		}
		if((sbConnected & 0x01) == 0 && beaconID == 253)
			printf("SB1 connected\n");
		if((sbConnected & 0x02) == 0 && beaconID == 254)
			printf("SB2 connected\n");
		// mark beacon as connected
		sbConnected = sbConnected | (beaconID == 253 ? 0x01 : 0x02);
	} else { // without answer, mark beacon as disconnected
		if((sbConnected & 0x01) && beaconID == 253)
			printf("SB1 disconnected\n");
		if((sbConnected & 0x02) && beaconID == 254)
			printf("SB2 disconnected\n");
		sbConnected = sbConnected & (beaconID == 253 ? 0x02 : 0x01);
	}
}

// ask for any robot to join the system
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

		printf("new robot connected : id=%d, uid=%d\n", id, uid);
	};
}

static void masterBeaconTask(void) {
	int i, j;
	dwt_setrxaftertxdelay(0);

	// clear restartMB flag
	restartMB = 0;

	// wait a bit for serial over USB to be available
	chThdSleepMilliseconds(500);

	// generate a session ID
	radioBuffer[0] = 0;
	radioBuffer[1] = 0;
	decaSend(2, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
	i = dwt_readtxtimestamplo32();
	sessionID = (i >> 8) + (i >> 16);

	printf("sessionID = %d\n", sessionID);

	while(restartMB == 0) {
		// send start-of-frame
		sendSOF();

		// retrieve slave beacon measurements
		readSlaveBeacon(253);
		readSlaveBeacon(254);

		// compute robots locations if all the beacons are available
		if(sbConnected == 0x03) {
			trilateralizeRobots();
		} else {
			for (i = 0; i < MAX_CONNECTED_ROBOTS; i++) {
				robots[i].x = 0;
				robots[i].y = 0;
			}
		}

		// send radio event (new distances and robots locations available)
		chEvtBroadcastFlags(&radioEvent, EVENT_MASK(0));

		// send data to the robots and measure distances
		dwt_setrxaftertxdelay(POLL_TO_RESP_RX);
		i=0;
		while(i < MAX_CONNECTED_ROBOTS && robotIDs[i] != 0) {
			dwt_setdelayedtrxtime((sofTS  + (i + 1)*3*TIMESLOT_LENGTH*MS_TO_DWT) >> 8);
			robots[robotIDs[i] - 1].mbDist =
				rangeRobot(robotIDs[i], serializeRobotData(&radioBuffer[2], robotIDs[i]));
			// distance = 0 means robot didn't respond or an error happened
			if(robots[robotIDs[i] - 1].mbDist == 0) {
				printf("robot %d disconnected\n", robotIDs[i]);
				robotIDs[i] = 0; // mark robot as disconnected
			} else {
				robots[robotIDs[i] - 1].mbDist = robots[robotIDs[i] - 1].mbDist*robots[robotIDs[i] - 1].offsets->mbCoeff/1000;
				robots[robotIDs[i] - 1].mbDist += robots[robotIDs[i] - 1].offsets->mb;
				robots[robotIDs[i] - 1].status = radioBuffer[4];
				// if additional payload was sent to the robot
				if(robotIDs[i] == payloadID) {
					payloadID = 0;
					// signal that payload has been successfully sent
					chEvtBroadcastFlags(&payloadEvent, EVENT_MASK(0));
				}
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

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

	while(1) {
		// initialize decawave module
		decaInit();
		// Set response timeout
		dwt_setrxtimeout(RX_TIMEOUT);

		if(deviceUID == 0)
			masterBeaconTask();
		else
			slaveBeaconTask();
	}
}

void dumpConnectedDevices(BaseSequentialStream *chp, int argc, char **argv) {
	int i;
	(void) argc;
	(void) argv;

	if(sbConnected & 0x01)
		chprintf(chp, "SB 1 ... connected\n");
	else
		chprintf(chp, "SB 1 ... NOT connected\n");
	if(sbConnected & 0x02)
		chprintf(chp, "SB 2 ... connected\n");
	else
		chprintf(chp, "SB 2 ... NOT connected\n");

	for(i=0; i<MAX_CONNECTED_ROBOTS; i++)
		if(robotIDs[i] != 0)
			chprintf(chp, "Robot ID=%d, UID=%d\n", robotIDs[i], robots[robotIDs[i]-1].offsets->uid);
}

void startRadio(void) {
	chThdCreateStatic(waRadio, sizeof(waRadio), NORMALPRIO+1, radioThread, NULL);
}

void resetDate(void) {
	date = 0;
}

void restartRadio(void) {
	int j;

	restartMB = 1;
	for(j=0; j<MAX_CONNECTED_ROBOTS; j++)
		robotIDs[j] = 0;

	chEvtBroadcastFlags(&deca_event, EVENT_MASK(0));
}
