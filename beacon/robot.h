#ifndef ROBOT_H
#define ROBOT_H

#include "nonvolatile.h"

struct robotData {
	// sent to the robot
	int16_t x;
	int16_t y;
	uint8_t flags;
	// not sent to the robot
	uint8_t status;
	struct distOffset *offsets;
	int16_t mbDist;
	int16_t sb1Dist;
	int16_t sb2Dist;
};

/* data about robots status and goal, robot <ID> data = robots[ID] */
extern struct robotData robots[];

/* ID of the robot to send payload to (or 0) */
extern int payloadID;

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

/* start dance, USAGE : dance */
void startDance(BaseSequentialStream *chp, int argc, char **argv);
/* stop dance, USAGE : stop */
void stopDance(BaseSequentialStream *chp, int argc, char **argv);

/* send and store points of the dance to a robot (up to 6 points at once)
 * USAGE : moves <ROBOT ID> <DATE> <X> <Y> <ANGLE> <START RADIUS> <END RADIUS> <DATE> ... */
void storeMoves(BaseSequentialStream *chp, int argc, char **argv);
/* send and store colors of the dance to a robot (up to 6 points at once)
 * USAGE : moves <ROBOT ID> <DATE> <H> <S> <V> <FADE TIME> <DATE> <H> ... */
void storeColors(BaseSequentialStream *chp, int argc, char **argv);

/* clear any previously stored data on a robot
 * USAGE : clear <ROBOT ID> */
void clearStoredData(BaseSequentialStream *chp, int argc, char **argv);
/* write previously stored data on a robot in its flash
 * USAGE : flash <ROBOT ID> */
void writeStoredData(BaseSequentialStream *chp, int argc, char **argv);

/* dump available data about a robot
 * USAGE : robot <ROBOT ID> */
void dumpRobotData(BaseSequentialStream *chp, int argc, char **argv);

#endif
