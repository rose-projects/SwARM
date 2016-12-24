#ifndef ROBOT_H
#define ROBOT_H

#include "non-volatile.h"

struct robotData {
	// sent to the robot
	uint8_t H;
	uint8_t S;
	uint8_t V;
	uint16_t x;
	uint16_t y;
	uint16_t goalX;
	uint16_t goalY;
	uint8_t goalSpeed;
	uint8_t flags;
	// not sent to the robot
	uint8_t status;
	struct distOffset *offsets;
	uint16_t mbDist;
	uint16_t sb1Dist;
	uint16_t sb2Dist;
};

/* data about robots status and goal, robot <ID> data = robots[ID] */
extern struct robotData robots[];

/* fill targetBuffer with a serialized version of the robot data to send.
 * Returns length of serialized data */
int serializeRobotData(uint8_t *targetBuffer, int robotID);

/* compute robots absolute locations and store them */
void trilateralizeRobots(void);

/* ############## Shell command callbacks ############## */

/* calibrate distance measurements between master beacon and a robot
 * USAGE: mbcal <ROBOT ID> <ACTUAL DISTANCE> (where ACTUAL DISTANCE is in cm) */
void mbCalibrate(BaseSequentialStream *chp, int argc, char **argv);
/* calibrate distance measurements between slave beacon 1 and a robot
 * USAGE: sb1cal <ROBOT ID> <ACTUAL DISTANCE> (where ACTUAL DISTANCE is in cm) */
void sb1Calibrate(BaseSequentialStream *chp, int argc, char **argv);
/* calibrate distance measurements between slave beacon 2 and a robot
 * USAGE: sb2cal <ROBOT ID> <ACTUAL DISTANCE> (where ACTUAL DISTANCE is in cm) */
void sb2Calibrate(BaseSequentialStream *chp, int argc, char **argv);

/* set the locations of the beacons
 * USAGE : beacon <SB1 X> <SB2 Y>
 * where SB1 X is the x coordinate of slave beacon 1 in cm
 * and SB2 Y is the y coordinate of the slave beacon 2 in cm */
void setBeaconPosition(BaseSequentialStream *chp, int argc, char **argv);

#endif
