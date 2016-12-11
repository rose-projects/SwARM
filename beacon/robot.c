#include <stdint.h>
#include <stdlib.h>
#include "ch.h"
#include "chprintf.h"
#include "chevents.h"

#include "../shared/radio-conf.h"
#include "robot.h"
#include "radio-comms.h"

// data about robots status and goal, robot <ID> data = robots[ID]
struct robotData robots[MAX_ROBOT_ID];

/* beacons position :
 * master beacon is at origin (0,0)
 * slave beacon 1 is at (sb1X, 0)
 * slave beacon 2 is at (0, sb2Y) */
int sb1X = 500;
int sb2Y = 500;

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

static void computeTrilateralisation(uint16_t mbR, uint16_t sb1R, uint16_t sb2R, uint16_t *x, uint16_t *y) {
	*x = ((mbR^2) - (sb1R^2) + (sb1X^2))/(2*sb1X);
	*y = ((mbR^2) - (sb2R^2) + (sb2Y^2))/(2*sb2Y);
}

void trilateralizeRobots(void) {
	int i;
	struct robotData *robot;

	for(i=0; i<MAX_ROBOT_ID; i++) {
		robot = &robots[i];

		// check all distances has been correctly measured
		if(robot->mbDist != 0 && robot->sb1Dist != 0 && robot->sb2Dist != 0) {
			computeTrilateralisation(robot->mbDist, robot->sb1Dist, robot->sb2Dist, &robot->x, &robot->y);
		}
	}
}

static int checkCalibrate(BaseSequentialStream *chp, int argc, char **argv, int *dist, int *id) {
	if(argc == 2 && (*dist= atoi(argv[0])) > 0 && atoi(argv[0]) < MAX_ROBOT_ID && (*id = atoi(argv[1]) > 0)) {
		return 0;
	} else {
		chprintf(chp, "USAGE : cal <ROBOT ID> <ACTUAL DISTANCE> (where ACTUAL DISTANCE is in cm)\n");
		return -1;
	}
}
static void calibrateRobot(BaseSequentialStream *chp, int expectedDistance, uint16_t *dist, int16_t *offset) {
	event_listener_t evt_listener;
	int averageDistance = 0, i;

	chEvtRegisterMask(&radio_event, &evt_listener, EVENT_MASK(0));
	chprintf(chp, "The 3 beacons must be connected, don't move the robot\nCalibrating ...");

	for(i=0; i<40; i++) {
		// wait for a measure
		chEvtWaitAny(ALL_EVENTS);
		averageDistance += *dist - *offset;
	}
	*offset = expectedDistance - averageDistance/40;

	chprintf(chp, " done. offset = %d cm\n", *offset);
	chEvtUnregister(&radio_event, &evt_listener);
}

void mbCalibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].mbDist, &robots[robotID - 1].mbOffset);
}

void sb1Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].sb1Dist, &robots[robotID - 1].sb1Offset);
}

void sb2Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance, robotID;

	if(checkCalibrate(chp, argc, argv, &expectedDistance, &robotID) < 0)
		return;

	calibrateRobot(chp, expectedDistance, &robots[robotID -1].sb2Dist, &robots[robotID - 1].sb2Offset);
}
