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
		} else {
			robot->x = 0;
			robot->y = 0;
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

	// clear flags
	robots[id - 1].flags = 0;

	return result;
}

static int checkCalibrate(BaseSequentialStream *chp, int argc, char **argv, int *dist1, int *dist2, int *id) {
	if(argc == 3 && (*dist1 = atoi(argv[1])) > 0 && (*dist2 = atoi(argv[2])) > 0 && *dist1 != *dist2
		&& atoi(argv[0]) < MAX_CONNECTED_ROBOTS && (*id = atoi(argv[0]) > 0))
	{
		return 0;
	} else {
		chprintf(chp, "USAGE : **cal ID ACTUAL_DISTANCE_1 ACTUAL_DISTANCE_2 (where ACTUAL_DISTANCE_X is in cm)\n");
		return -1;
	}
}

static void calibrateRobot(BaseSequentialStream *chp, int expectedDistance1, int expectedDistance2,
	int16_t *dist, int16_t *offset, int16_t *coeff)
{
	event_listener_t evt_listener;
	int averageDistance1 = 0, averageDistance2 = 0, i;

	chEvtRegisterMask(&radioEvent, &evt_listener, EVENT_MASK(0));

	if(*coeff == 0) {
		*coeff = 1000;
		return;
	}

	chprintf(chp, "The 3 beacons must be connected, don't move the robot\nCalibrating 1st point ...");
	for(i=0; i<40; i++) {
		// wait for a measure
		chEvtWaitAny(ALL_EVENTS);
		averageDistance1 += ((*dist - *offset)*1000)/(*coeff);
	}
	chprintf(chp, " OK\n");

	for(i=10; i>0; i--) {
		chprintf(chp, "Next measure in %ds\n", i);
		chThdSleepMilliseconds(1000);
	}

	chprintf(chp, "Calibrating 2nd point ...");
	for(i=0; i<40; i++) {
		// wait for a measure
		chEvtWaitAny(ALL_EVENTS);
		averageDistance2 += ((*dist - *offset)*1000)/(*coeff);
	}

	*coeff = ((expectedDistance2 - expectedDistance1)*40000)/(averageDistance2 - averageDistance1);
	*offset = expectedDistance1 - averageDistance1*(*coeff)/40000;

	chprintf(chp, " OK. Offset = %d cm, coeff = %d\n", *offset, *coeff);
	chEvtUnregister(&radioEvent, &evt_listener);
}

void mbCalibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance1, expectedDistance2, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance1, &expectedDistance2, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;
	offset.mbCoeff = robots[robotID -1].offsets->mbCoeff;
	offset.sb1Coeff = robots[robotID -1].offsets->sb1Coeff;
	offset.sb2Coeff = robots[robotID -1].offsets->sb2Coeff;

	calibrateRobot(chp, expectedDistance1, expectedDistance2, &robots[robotID -1].mbDist, &offset.mb, &offset.mbCoeff);
	writeOffset(&offset);
}

void sb1Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance1, expectedDistance2, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance1, &expectedDistance2, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;
	offset.mbCoeff = robots[robotID -1].offsets->mbCoeff;
	offset.sb1Coeff = robots[robotID -1].offsets->sb1Coeff;
	offset.sb2Coeff = robots[robotID -1].offsets->sb2Coeff;

	calibrateRobot(chp, expectedDistance1, expectedDistance2, &robots[robotID -1].sb1Dist, &offset.sb1, &offset.sb1Coeff);
	writeOffset(&offset);
}

void sb2Calibrate(BaseSequentialStream *chp, int argc, char **argv) {
	int expectedDistance1, expectedDistance2, robotID;
	struct distOffset offset;

	if(checkCalibrate(chp, argc, argv, &expectedDistance1, &expectedDistance2, &robotID) < 0)
		return;

	// load current offsets in RAM
	offset.uid = robots[robotID -1].offsets->uid;
	offset.mb = robots[robotID -1].offsets->mb;
	offset.sb1 = robots[robotID -1].offsets->sb1;
	offset.sb2 = robots[robotID -1].offsets->sb2;
	offset.mbCoeff = robots[robotID -1].offsets->mbCoeff;
	offset.sb1Coeff = robots[robotID -1].offsets->sb1Coeff;
	offset.sb2Coeff = robots[robotID -1].offsets->sb2Coeff;

	calibrateRobot(chp, expectedDistance1, expectedDistance2, &robots[robotID -1].sb2Dist, &offset.sb2, &offset.sb2Coeff);
	writeOffset(&offset);
}

/* set the locations of the beacons
* USAGE : beacon <SB1 X> <SB2 Y>
* where SB1 X is the x coordinate of slave beacon 1 in cm
* and SB2 Y is the y coordinate of the slave beacon 2 in cm */
void setBeaconPosition(BaseSequentialStream *chp, int argc, char **argv) {
	int x, y;

	if(argc != 2) {
		chprintf(chp, "USAGE : beacon SB1_X SB2_Y\n");
		return;
	}

	x = atoi(argv[0]);
	y = atoi(argv[1]);
	if(x != 0 && y != 0) {
		sb1X = x;
		sb2Y = y;
		chprintf(chp, "OK\n");
	} else {
		chprintf(chp, "KO\n");
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
		int dataLength = (argc - 1) / 6;

		for(i=0; i<dataLength; i++) {
			int date = atoi(argv[i*6 + 1]);
			int x = atoi(argv[i*6 + 2]), y = atoi(argv[i*6 + 3]);
			int angle = (atoi(argv[i*6 + 4]) << 8)/360;
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
		int dataLength = (argc - 1) / 5;

		for(i=0; i<dataLength; i++) {
			int date = atoi(argv[i*5 + 1]);
			int h = atoi(argv[i*5 + 2]), s = atoi(argv[i*5 + 3]), v = atoi(argv[i*5 + 4]);
			int fadeTime = atoi(argv[i*5 + 5]);
			if(fadeTime == 0)
				fadeTime = 1;

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

void dumpRobotData(BaseSequentialStream *chp, int argc, char **argv) {
	if(argc == 1 && atoi(argv[0]) != 0 && atoi(argv[0]) < MAX_CONNECTED_ROBOTS) {
		int id = atoi(argv[0]) - 1;

		if(robots[id].offsets != NULL) {
			chprintf(chp, "UID      = %d\n", robots[id].offsets->uid);
			chprintf(chp, "offsets  = %d, %d, %d\n", robots[id].offsets->mb, robots[id].offsets->sb1, robots[id].offsets->sb2);
		}

		chprintf(chp, "x        = %d cm\n", robots[id].x);
		chprintf(chp, "y        = %d cm\n", robots[id].y);
		chprintf(chp, "MB dist  = %d cm\n", robots[id].mbDist);
		chprintf(chp, "SB1 dist = %d cm\n", robots[id].sb1Dist);
		chprintf(chp, "SB2 dist = %d cm\n", robots[id].sb2Dist);

		chprintf(chp, "status  = %d", robots[id].status);
		switch(robots[id].status & RB_STATUS_BATT) {
		case BATTERY_HIGH:
			printf(" (battery: high)\n");
			break;
		case BATTERY_OK:
			printf(" (battery: ok)\n");
			break;
		case BATTERY_LOW:
			printf(" (battery: low)\n");
			break;
		case BATTERY_VERYLOW:
			printf(" (battery: very low !)\n");
			break;
		}
	} else {
		chprintf(chp, "KO\n");
	}
}
