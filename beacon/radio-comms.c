#include "ch.h"
#include "chvt.h"
#include "chprintf.h"

#include "../shared/usb-config.h"
#include "../shared/deca-functions.h"
#include "../shared/decadriver/deca_device_api.h"
#include "../shared/decadriver/deca_regs.h"

//#define DEVICE_UID 0 // master beacon
#define DEVICE_UID 253 // slave beacon 1
//#define DEVICE_UID 254 // slave beacon 2

#define MS_TO_DWT ((int64_t) 1000*UUS_TO_DWT_TIME)

#define POLL_TO_RESP_DLY 600*UUS_TO_DWT_TIME // Delay between ranging poll RX and response TX
#define POLL_TO_RESP_RX 400 // Delay between poll and RX activation
#define RX_TIMEOUT 2000 // RX timeout in us
#define SYNC_RX_TIMEOUT 50000 // time waiting for start-of-frame in us
#define AHEAD_OF_TX_MARGIN 200*UUS_TO_DWT_TIME // required time between RX enable and actual RX
#define FRAME_LENGTH 100 // total length of a frame in ms
#define TIMESLOT_LENGTH 2 // time slot length in ms
// delay between start-of-frame and device time slot
#if DEVICE_UID == 253
	#define TIMESLOT_DELAY 2
#else
	#define TIMESLOT_DELAY 4
#endif

#define SOF_MSG_ID 0x23
#define BEACON_READ_MSG_ID 0x32
#define RANGING_MSG_ID 0x42

// RX/TX buffer
#define RADIO_BUF_LEN 100
static uint8_t radio_buffer[RADIO_BUF_LEN];

// timestamp of the last start-of-frame
static int64_t sofTS = -1;
static systime_t sofSystime = -1;
// for slave beacon : 1 if beacon is known by master beacon
static int isRegistered = 0;

#define MAX_CONNECTED_ROBOTS 48
#define MAX_ROBOT_ID 50
// connected robots IDs
static uint8_t robotIDs[MAX_CONNECTED_ROBOTS];
// measured robot distances, in cm, robot <ID> distance = robotDistances[ID]
static int16_t robotDistances[MAX_ROBOT_ID];

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
		radio_buffer[msg_index++] = robotDistances[robotIDs[i]-1];
		radio_buffer[msg_index++] = robotDistances[robotIDs[i]-1] >> 8;
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
		robotDistances[ret] = 0;

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
		distanceInCm = distance;
		return distanceInCm;
	}
	return 0;
}

static int slaveBeaconRead(int msgID, int addr, uint64_t timeInFrame) {
	int ret;

	// if we have to wait for too long
	if(chVTGetSystemTime() - sofSystime < timeInFrame - MS2ST(10))
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
				dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);
				robotDistances[robotIDs[i] - 1] = rangeRobot(robotIDs[i], 0);
				i++;
				timeInFrame += TIMESLOT_LENGTH;
			}
			switchToChannel(MB_CHANNEL);
		}
	}
}
