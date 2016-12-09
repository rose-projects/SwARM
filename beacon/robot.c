#include <stdint.h>

#include "radio-conf.h"
#include "robot.h"

struct robotData robots[MAX_CONNECTED_ROBOTS];

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

void trilateralizeRobots(uint16_t* mbDists, uint16_t* sb1Dists, uint16_t* sb2Dists) {
	int i;
	for(i=0; i<MAX_ROBOT_ID; i++) {
		// check all distances has been correctly measured
		if(mbDists != 0 && sb1Dists != 0 && sb2Dists != 0) {
			computeTrilateralisation(mbDists[i], sb1Dists[i], sb2Dists[i], &robots[i].x, &robots[i].y);
		}
	}
}
