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
#include "radio-comms.h"

// event triggered when new data has been received
EVENTSOURCE_DECL(radioEvent);

// RX/TX buffer
#define RADIO_BUF_LEN 100
static uint8_t radioBuffer[RADIO_BUF_LEN];

static int deviceID  = 0;
static int sessionID  = -1;

// timestamp of the last start-of-frame
static int64_t sofTS = -1;
static systime_t sofSystime = -1;

// registered position (indicates when to listen)
static int registered = 0;

// robot data to receive/send
struct robotData radioData;

static void parseSOF(int sofLength) {
	int i = 4;

	// get start-of-frame reception time
	sofTS = getRXtimestamp();
	sofSystime = chVTGetSystemTime();

	// check sessionID didn't change
	if(sessionID == -1)
		sessionID = radioBuffer[2];
	else if(sessionID != radioBuffer[2]) {
		// if it changed, restart sync
		registered = 0;
		sessionID = radioBuffer[2];
		return;
	}

	// search for the robot's ID in the list
	while(i < sofLength && radioBuffer[i] != deviceID)
		i++;

	// if found, robot is known to be active by master beacon
	if(i != sofLength)
		registered = i - 3;
	else
		registered = 0;
}

static void parseRadioData(void) {
	radioData.H = radioBuffer[2];
	radioData.S = radioBuffer[3];
	radioData.V = radioBuffer[4];
	radioData.x = radioBuffer[5];
	radioData.x &= radioBuffer[6] << 8;
	radioData.y = radioBuffer[7];
	radioData.y &= radioBuffer[8] << 8;
	radioData.goalX = radioBuffer[9];
	radioData.goalX &= radioBuffer[10] << 8;
	radioData.goalY = radioBuffer[11];
	radioData.goalY &= radioBuffer[12] << 8;
	radioData.goalSpeed = radioBuffer[13];
	radioData.flags = radioBuffer[14];
	radioBuffer[2] = radioData.status;
}

static int messageRead(int msgID, int addr, uint64_t timeInFrame) {
	int ret;

	sleepUntil(sofSystime, timeInFrame - 1);
	dwt_setdelayedtrxtime((sofTS  + timeInFrame*MS_TO_DWT - AHEAD_OF_TX_MARGIN) >> 8);

	ret = decaReceive(RADIO_BUF_LEN, radioBuffer, DWT_START_RX_DELAYED);
	if(ret > 0) {
		if(radioBuffer[0] == msgID && radioBuffer[1] == addr)
			return ret;
	}

	return -1;
}

static void synchronizeRadio(void) {
	int ret, frameCounter = 0, retryDelay = 0;

	// listen to master beacon
	switchToChannel(MB_CHANNEL);
	dwt_setrxtimeout(SYNC_RX_TIMEOUT);

	// not synchronized : turn on red LED and turn off green LED
	palSetPad(GPIOF, GPIOF_STAT3);
	palClearPad(GPIOF, GPIOF_CAM_PWR);

	sofTS = -1;
	while(sofTS == -1 || registered == 0) {
		if((ret = decaReceive(RADIO_BUF_LEN, radioBuffer, DWT_START_RX_IMMEDIATE)) < 0) {
			// no valid message received
			sofTS = -1;
			// allow module to cool down ten times longer
			chThdSleepMilliseconds(SYNC_RX_TIMEOUT/100);
			continue;
		}

		// message is a start of frame
		if(radioBuffer[0] == SOF_MSG_ID && radioBuffer[1] == 0xFF) {
			parseSOF(ret);

			// synchronized if registered
			if(registered)
				continue;

			// retry to register after a random number of frames
			if(frameCounter == retryDelay) {
				ret = messageRead(NEW_ROBOT_MSG_ID, 0xFF, FRAME_LENGTH - TIMESLOT_LENGTH);
				if(ret == 3 && radioBuffer[2] != 0) {
					uint32_t uid = dwt_getpartid();
					deviceID = radioBuffer[2];

					radioBuffer[1] = 0;
					radioBuffer[2] = uid;
					radioBuffer[3] = uid >> 8;
					radioBuffer[4] = uid >> 16;
					decaSend(5, radioBuffer, 1, DWT_START_TX_IMMEDIATE);
				}

				frameCounter = 0;
				ret = dwt_readrxtimestamplo32();
				retryDelay = ((ret >> 8) + (ret >> 16)) & 0x1F;
			}
			frameCounter++;

			sleepUntil(sofSystime, FRAME_LENGTH - 4);
		}
	}
	dwt_setrxtimeout(RX_TIMEOUT);

	// synchronized : turn off red LED and turn on green LED
	palClearPad(GPIOF, GPIOF_STAT3);
	palSetPad(GPIOF, GPIOF_CAM_PWR);
}

static void rangingResponse(int sendStatus) {
	// Retrieve poll reception timestamp
	uint64_t rxTS = getRXtimestamp();

	// set response message transmission time
	dwt_setdelayedtrxtime((rxTS + POLL_TO_RESP_DLY) >> 8);

	// send the response message
	radioBuffer[1] = 0;
	radioBuffer[2] = rxTS;
	radioBuffer[3] = rxTS >> 8;
	radioBuffer[4] = radioData.status;
	decaSend(4 + sendStatus, radioBuffer, 1, DWT_START_TX_DELAYED);
}

static THD_WORKING_AREA(waRadio, 256);
static THD_FUNCTION(radioThread, th_data) {
	event_listener_t evt_listener;
	int ret;

	(void) th_data;
	chRegSetThreadName("Radio");

	// initialize decawave module
	decaInit();
	// Set expected response's delay and timeout
	dwt_setrxaftertxdelay(POLL_TO_RESP_RX);
	dwt_setrxtimeout(RX_TIMEOUT);

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

	while(1) {
		// if last start-of-frame time isn't known or robot isn't registered
		if(sofTS == -1 || registered == 0)
			synchronizeRadio();
		else {
			if((ret = messageRead(SOF_MSG_ID, 0xFF, FRAME_LENGTH)) > 0)
				parseSOF(ret);

			// if something went wrong, restart synchronisation
			if(registered == 0 || ret < 0) {
				sofTS = -1;
				chThdSleepMilliseconds(FRAME_LENGTH - 4);
				continue;
			}
		}

		if(messageRead(RANGING_MSG_ID, deviceID, (registered+2)*TIMESLOT_LENGTH) == 15) {
			parseRadioData();
			rangingResponse(1);
			// send radio event (new data available)
		    chEvtBroadcastFlags(&radioEvent, EVENT_MASK(0));
		} else {
			sofTS = -1;
		}
		switchToChannel(SB1_CHANNEL);
		if(messageRead(RANGING_MSG_ID, deviceID, (registered+4)*TIMESLOT_LENGTH) > 0) {
			rangingResponse(0);
		}
		switchToChannel(SB2_CHANNEL);
		if(messageRead(RANGING_MSG_ID, deviceID, (registered+6)*TIMESLOT_LENGTH) > 0) {
			rangingResponse(0);
		}
		switchToChannel(MB_CHANNEL);
	}
}

void startRadio(void) {
	chThdCreateStatic(waRadio, sizeof(waRadio), NORMALPRIO+1, radioThread, NULL);
}
