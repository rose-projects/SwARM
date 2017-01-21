#include <stdint.h>
#include <stdlib.h>
#include "ch.h"
#include "chevents.h"

#include "../shared/radio-conf.h"
#include "../shared/usb-config.h"
#include "robot.h"
#include "radio-comms.h"

// data about robots status and goal, robot <ID> data = robots[ID]
struct robotData robots[MAX_CONNECTED_ROBOTS];

/* beacons position :
 * master beacon is at origin (0,0)
 * slave beacon 1 is at (sb1X, 0)
 * slave beacon 2 is at (0, sb2Y) */
int sb1X = 100;
int sb2Y = 100;

int serializeRobotData(uint8_t *targetBuffer, int robotID) {
	struct robotData *robot = &robots[robotID-1];

	targetBuffer[0] = robot->H;
	targetBuffer[1] = robot->S;
	targetBuffer[2] = robot->V;
	targetBuffer[3] = robot->x;
	targetBuffer[4] = robot->x >> 8;
	targetBuffer[5] = robot->y;
	targetBuffer[6] = robot->y >> 8;
	targetBuffer[7] = robot->goalX;
	targetBuffer[8] = robot->goalX >> 8;
	targetBuffer[9] = robot->goalY;
	targetBuffer[10] = robot->goalY >> 8;
	targetBuffer[11] = robot->goalSpeed;
	targetBuffer[12] = robot->flags;
	return 13;
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
			printf(" %d %d %d", robot->x, robot->y, robot->sb1Dist);
		}
	}
	printf("\n");
}

static int checkCalibrate(BaseSequentialStream *chp, int argc, char **argv, int *dist, int *id) {
	if(deviceUID != 0) {
		chprintf(chp, "available only on master beacon\n");
		return -1;
	}

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

	if(deviceUID != 0) {
		chprintf(chp, "available only on master beacon\n");
		return;
	}

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
