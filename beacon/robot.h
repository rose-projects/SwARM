#ifndef ROBOT_H
#define ROBOT_H

struct robotData {
	uint8_t H;
	uint8_t S;
	uint8_t V;
	uint16_t x;
	uint16_t y;
	uint16_t goalX;
	uint16_t goalY;
	uint8_t goalSpeed;
	uint8_t flags;
};

/* fill targetBuffer with a serialized version of all the robot data.
 * Returns length of serialized data */
int serializeRobotData(uint8_t *targetBuffer, int robotID);

/* compute robots absolute locations and store them */
void trilateralizeRobots(uint16_t* mbDists, uint16_t* sb1Dists, uint16_t* sb2Dists);


#endif
