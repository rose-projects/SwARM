#include <stdint.h>
#include <stdlib.h>
#include "ch.h"
#include "chevents.h"

#include "usbconf.h"
#include "../shared/radioconf.h"
#include "robot.h"
#include "radiocomms.h"

// data about robots status and goal, robot <ID> data = robots[ID]
struct robotData robots[MAX_CONNECTED_ROBOTS];

/* beacons position :
 * master beacon is at origin (0,0)
 * slave beacon 1 is at (sb1X, 0)
 * slave beacon 2 is at (0, sb2Y) */
int sb1X = 100;
int sb2Y = 100;

int danceEnable = 0;

static uint8_t payloadBuffer[64];
static int payloadSize = 0;
int payloadID = 0;

int serializeRobotData(uint8_t *targetBuffer, int robotID) {
	struct robotData *robot = &robots[robotID-1];
	int i, size = 5;
	targetBuffer[0] = robot->x;
	targetBuffer[1] = robot->x >> 8;
	targetBuffer[2] = robot->y;
	targetBuffer[3] = robot->y >> 8;
	targetBuffer[4] = robot->flags;

	if(danceEnable) {
		targetBuffer[4] |= RB_FLAGS_DEN;
	}

	if(robotID == payloadID) {
		for(i=0; i<payloadSize; i++)
			targetBuffer[i+5] = payloadBuffer[i];

		size += payloadSize;
	}
	return size;
}

static void computeTrilateralisation(int mbR, int sb1R, int sb2R, int16_t *x, int16_t *y) {
	*x = ((mbR*mbR) - (sb1R*sb1R) + (sb1X*sb1X))/(2*sb1X);
	*y = ((mbR*mbR) - (sb2R*sb2R) + (sb2Y*sb2Y))/(2*sb2Y);
}

void trilateralizeRobots(void) {
	int i;
	struct robotData *robot;
	printf("POS :");
	for(i=0; i<MAX_CONNECTED_ROBOTS; i++) {
		robot = &robots[i];

		// check all distances has been correctly measured
		if(robot->mbDist != 0 && robot->sb1Dist != 0 && robot->sb2Dist != 0) {
			computeTrilateralisation(robot->mbDist, robot->sb1Dist, robot->sb2Dist, &robot->x, &robot->y);
			printf(" %d %d", robot->x, robot->y);
		}
	}
	printf("\n");
}

static int sendPayload(int id, int size) {
	event_listener_t evt_listener;
	int result;

	payloadSize = size;
	payloadID = id;

	chEvtRegisterMask(&payloadEvent, &evt_listener, EVENT_MASK(0));
	result = chEvtWaitAnyTimeout(ALL_EVENTS, CH_CFG_ST_FREQUENCY); // timeout = 1 sec
	chEvtUnregister(&payloadEvent, &evt_listener);

	return result;
}

static int checkCalibrate(BaseSequentialStream *chp, int argc, char **argv, int *dist, int *id) {
	if(argc == 2 && (*dist = atoi(argv[1])) > 0 && atoi(argv[0]) < MAX_CONNECTED_ROBOTS && (*id = atoi(argv[0]) > 0)) {
		return 0;
	} else {
		chprintf(chp, "USAGE : **cal <ROBOT ID> <ACTUAL DISTANCE> (where ACTUAL DISTANCE is in cm)\n");
		return -1;
	}
}
static void calibrateRobot(BaseSequentialStream *chp, int expectedDistance, int16_t *dist, int16_t *offset) {
	event_listener_t evt_listener;
	int averageDistance = 0, i;

	chEvtRegisterMask(&radioEvent, &evt_listener, EVENT_MASK(0));
	chprintf(chp, "The 3 beacons must be connected, don't move the robot\nCalibrating ...");

	for(i=0; i<40; i++) {
		// wait for a measure
		chEvtWaitAny(ALL_EVENTS);
		averageDistance += *dist - *offset;
	}
	*offset = expectedDistance - averageDistance/40;

	chprintf(chp, " done. Offset = %d cm\n", *offset);
	chEvtUnregister(&radioEvent, &evt_listener);
}

void mbCalibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].mbDist, &offset.mb);
	writeOffset(&offset);
}

void sb1Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].sb1Dist, &offset.sb1);
	writeOffset(&offset);
}

void sb2Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].sb2Dist, &offset.sb2);
	writeOffset(&offset);
}

/* set the locations of the beacons
* USAGE : beacon <SB1 X> <SB2 Y>
* where SB1 X is the x coordinate of slave beacon 1 in cm
* and SB2 Y is the y coordinate of the slave beacon 2 in cm */
void setBeaconPosition(BaseSequentialStream *chp, int argc, char **argv) {
	int x, y;

	if(argc != 2) {
		chprintf(chp, "USAGE : beacon <SB1 X> <SB2 Y>\n");
		return;
	}

	x = atoi(argv[0]);
	y = atoi(argv[1]);
	if(x != 0 && y != 0) {
		sb1X = x;
		sb2Y = y;
		chprintf(chp, "OK\n");
	} else {
		chprintf(chp, "SB1 X and SB2 Y can't be 0\n");
	}
}

void startDance(BaseSequentialStream *chp, int argc, char **argv) {
	(void) argc;
	(void) argv;

	resetDate();
	danceEnable = 1;
	chprintf(chp, "OK\n");
}

void stopDance(BaseSequentialStream *chp, int argc, char **argv) {
	(void) argc;
	(void) argv;

	danceEnable = 0;
	chprintf(chp, "OK\n");
}

void clearStoredData(BaseSequentialStream *chp, int argc, char **argv) {
	if(argc == 1 && atoi(argv[0]) != 0) {
		int id = atoi(argv[0]);
		robots[id - 1].flags = RB_FLAGS_CLR;
		if(sendPayload(id, 0) != 0 && (robots[id - 1].status & RB_STATUS_WOK) == 0) {
			chprintf(chp, "OK\n");
			return;
		}
	}
	chprintf(chp, "KO\n");
}

void storeMoves(BaseSequentialStream *chp, int argc, char **argv) {
	// check parameters : maximum is 6 points at once
	if(argc > 1 && (argc - 1) % 6 == 0 && argc < 38 && atoi(argv[0]) != 0) {
		int i, id = atoi(argv[0]);
		int dataLength = (argc - 1) % 6;

		for(i=0; i<dataLength; i++) {
			int date = atoi(argv[i*6 + 1]);
			int x = atoi(argv[i*6 + 2]), y = atoi(argv[i*6 + 3]);
			int angle = atoi(argv[i*6 + 4]);
			int rs = atoi(argv[i*6 + 5]), re = atoi(argv[i*6 + 6]);

			payloadBuffer[i*11] = date;
			payloadBuffer[i*11 + 1] = date >> 8;
			payloadBuffer[i*11 + 2] = x;
			payloadBuffer[i*11 + 3] = x >> 8;
			payloadBuffer[i*11 + 4] = y;
			payloadBuffer[i*11 + 5] = y >> 8;
			payloadBuffer[i*11 + 6] = angle;
			payloadBuffer[i*11 + 7] = rs;
			payloadBuffer[i*11 + 8] = rs >> 8;
			payloadBuffer[i*11 + 9] = re;
			payloadBuffer[i*11 + 10] = re >> 8;
		}

		robots[id - 1].flags = RB_FLAGS_PTSTR;
		if(sendPayload(id, dataLength*11) != 0) {
			chprintf(chp, "OK\n");
			return;
		}
	}
	chprintf(chp, "KO\n");
}

void storeColors(BaseSequentialStream *chp, int argc, char **argv) {
	// check parameters : maximum is 6 color points at once
	if(argc > 1 && (argc - 1) % 5 == 0 && argc < 32 && atoi(argv[0]) != 0) {
		int i, id = atoi(argv[0]);
		int dataLength = (argc - 1) % 5;

		for(i=0; i<dataLength; i++) {
			int date = atoi(argv[i*5 + 1]);
			int h = atoi(argv[i*5 + 2]), s = atoi(argv[i*5 + 3]), v = atoi(argv[i*5 + 4]);
			int fadeTime = atoi(argv[i*5 + 5]);

			payloadBuffer[i*6] = date;
			payloadBuffer[i*6 + 1] = date >> 8;
			payloadBuffer[i*6 + 2] = h;
			payloadBuffer[i*6 + 3] = s;
			payloadBuffer[i*6 + 4] = v;
			payloadBuffer[i*6 + 5] = fadeTime;
		}

		robots[id - 1].flags = RB_FLAGS_CLSTR;
		if(sendPayload(id, dataLength*6) != 0) {
			chprintf(chp, "OK\n");
			return;
		}
	}
	chprintf(chp, "KO\n");
}

void writeStoredData(BaseSequentialStream *chp, int argc, char **argv) {
	if(argc == 1 && atoi(argv[0]) != 0) {
		int id = atoi(argv[0]);
		robots[id - 1].flags = RB_FLAGS_WF;
		if(sendPayload(id, 0) != 0 && (robots[id - 1].status & RB_STATUS_WOK)) {
			chprintf(chp, "OK\n");
			return;
		}
	}
	chprintf(chp, "KO\n");
}
