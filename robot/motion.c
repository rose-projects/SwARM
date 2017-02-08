#include <math.h>
#include <stdint.h>

#include "trigo.h"
#include "ch.h"
#include "hal.h"
#include "dance.h"
#include "RTT/SEGGER_RTT.h"
#include "codingwheels.h"
#include "pid.h"
#include "position.h"
#include "radiocomms.h"
#include "../shared/radioconf.h"

#define SIGN(x) (x > 0 ? 1 : -1)

typedef struct {
	float x;
	float y;
} point_t;

struct interpoints {
	point_t tan1;
	point_t tan2;
	float alpha1;
	float alpha2;
};

static float distance(point_t a, point_t b) {
	return sqrt(pow((b.x - a.x), 2) + pow((b.y - a.y), 2));
}

static point_t dir2vec(float dir) {
	point_t result = {mcos(dir), msin(dir)};
	return result;
}

static float scal(point_t a, point_t b) {
	return a.x*b.x + a.y*b.y;
}

static point_t subs(point_t from, point_t to) {
	point_t result;
	result.x = to.x - from.x;
	result.y = to.y - from.y;
	return result;
}

static float det(point_t a, point_t b) {
	return a.x*b.y - b.x*a.y;
}

static void computeTangent(point_t *center1, int r1, point_t *center2, int r2, int i, point_t *tan1, point_t *tan2) {
	point_t h;
	int l1 = i % 2 ? 1 : -1;
	int l2 = i > 1 ? 1 : -1;
	int innerTan = -l1*l2;

	h.x = (r1*center2->x + innerTan*r2*center1->x) / (r1 + innerTan*r2);
	h.y = (r1*center2->y + innerTan*r2*center1->y) / (r1 + innerTan*r2);

	tan1->x = center1->x + (r1*r1*(h.x-center1->x) + l1*r1*(h.y-center1->y)
		* sqrt(pow((h.x - center1->x), 2) + pow((h.y - center1->y), 2) - r1*r1)) / pow(distance(h, *center1), 2);
	tan1->y = center1->y + (r1*r1*(h.y-center1->y) - l1*r1*(h.x-center1->x)
		* sqrt(pow((h.x-center1->x), 2) + pow((h.y-center1->y), 2) - r1*r1)) / pow(distance(h, *center1), 2);

	tan2->x = center2->x + (r2*r2*(h.x-center2->x) - innerTan*l2*r2*(h.y-center2->y)
		* sqrt(pow((h.x-center2->x), 2) + pow((h.y-center2->y), 2) - r2*r2)) / pow(distance(h, *center2), 2);
	tan2->y = center2->y + (r2*r2*(h.y-center2->y) + innerTan*l2*r2*(h.x-center2->x)
		* sqrt(pow((h.x-center2->x), 2) + pow((h.y-center2->y), 2) - r2*r2)) / pow(distance(h, *center2), 2);
}

static void computeCenters(point_t *point, float dir, int radius, point_t *center1, point_t *center2) {
	float dirCos = mcos(dir);
	float dirSin = msin(dir);

	center1->x = point->x + radius*dirSin;
	center1->y = point->y - radius*dirCos;
	center2->x = point->x - radius*dirSin;
	center2->y = point->y + radius*dirCos;
}

static struct interpoints computeInterpoints(struct move *depMove, struct move *destMove) {
	struct interpoints result;
	int minimum = 0x7FFFFFFF;
	float tmp, depDet, destDet;
	float depDir = depMove->angle*M_PI/128, destDir = destMove->angle*M_PI/128;
	int startRadius = destMove->startRadius;
	int endRadius = destMove->endRadius;

	point_t tan1, tan2;
	point_t depCenters[2], destCenters[2];
	point_t dep = {depMove->x, depMove->y};
	point_t dest = {destMove->x, destMove->y};

	// both circles cannot be of the same radius because of the way we compute tangents
	if(startRadius == endRadius)
		startRadius++;

	computeCenters(&dep, depDir, startRadius, &depCenters[0], &depCenters[1]);
	computeCenters(&dest, destDir, endRadius, &destCenters[0], &destCenters[1]);

	for(int i=0; i<2; i++) { // try the 2 possible departure circles
		for(int j=0; j<2; j++) { // try the 2 possible destination circles
			depDet = det(subs(depCenters[i], dep), dir2vec(depDir));
			destDet = det(subs(destCenters[j], dest), dir2vec(destDir));

			for(int k = 0; k < 4; k++) { // try the 4 posible tangents
				computeTangent(&depCenters[i], startRadius, &destCenters[j], endRadius, k, &tan1, &tan2);

				// check if the tangent is the right one
				if(det(subs(depCenters[i], tan1), subs(tan1, tan2))*depDet > 0
					&& det(subs(destCenters[j], tan2), subs(tan1, tan2))*destDet > 0)
				{
					break;
				}
			}

			// the right tangent is the shorter one
			tmp = distance(tan1, tan2);
			if(tmp < minimum) {
				minimum = tmp;
				result.tan1 = tan1;
				result.tan2 = tan2;
			}
		}
	}

	// compute angles subtended by the arcs
	result.alpha1 = fabs(2*masin(distance(dep, result.tan1)/(2*startRadius)));
	if(scal(subs(dep, result.tan1), dir2vec(depDir)) < 0)
		result.alpha1 = 2*M_PI - result.alpha1;

	result.alpha2 = fabs(2*masin(distance(dest, result.tan2)/(2*endRadius)));
	if(scal(subs(result.tan2, dest), dir2vec(destDir)) < 0)
		result.alpha2 = 2*M_PI - result.alpha2;

	beginNewPID();
	return result;
}

// compute PID goals knowing the trajectory and current date
static void getGoal(float date, struct move *depMove, struct move *destMove, struct interpoints *r, float *dist, float *diff) {
	float depDir = depMove->angle*M_PI/128;
	point_t dep = {depMove->x, depMove->y};
	point_t dest = {destMove->x, destMove->y};

	// distances from the beginning of the move in cm
	float distDep = r->alpha1*destMove->startRadius; // distance after 1st arc
	float distStraight = distance(r->tan1, r->tan2) + distDep; // distance after straight line
	float distDest = r->alpha2*destMove->endRadius + distStraight; // distance after second arc

	// stay at the final location after the end of the move
	if(date > destMove->date)
		date = destMove->date;

	*dist = distDest*(date-depMove->date)/(destMove->date - depMove->date);
	*diff = 0;

	// compute diff between wheels in or after the 1st arc
	if(*dist < distDep) // robot is in the first arc
		*diff = *dist*WHEEL_DIST/destMove->startRadius*SIGN( det(dir2vec(depDir), subs(dep, r->tan1)) );
	else // robot is after the 1st arc
		*diff = distDep*WHEEL_DIST/destMove->startRadius*SIGN( det(dir2vec(depDir), subs(dep, r->tan1)) );

	if(*dist > distStraight) // robot is in the last arc
		*diff += (*dist - distStraight)*WHEEL_DIST/destMove->endRadius*SIGN( det(subs(r->tan1, r->tan2), subs(r->tan2, dest)) );
}

static int trajectoryUpdate = 0, resetPos = 0;

// a new move is at currentMove, trajectory must be recalculated
void updateInterpoints(void) {
	trajectoryUpdate = 1;
}

// set the robot at the origin of the dance
void resetPosition(void) {
	resetPos = 1;
}

// frequency of goal recalculation in Hz
#define GOALS_REFRESH_RATE 20

static THD_WORKING_AREA(waMotion, 512);
static THD_FUNCTION(motionThread, th_data) {
	(void) th_data;
	chRegSetThreadName("Motion");

	// current position
	float currentOrientation;
	// PID goals
	float dist, diff;
	// trajectory computed results
	struct interpoints currentInterpoints;
	// 1 if robot is dancing and trajectory data is valid (allows the robot to move)
	int dancing;
	// departure point of the current move
	struct move dep;

	while(1) {
		// set the robot at the origin of the dance if required
		// (stored at currentMove if dance isnt started)
		if(resetPos) {
			currentX = currentMove->x;
			currentY = currentMove->y;
			currentOrientation = currentMove->angle*M_PI/128;

			beginNewPID();
			resetPos = 0;
		}

		// force robot to stop if dance isn't enabled or battery is too low
		if((radioData.flags & RB_FLAGS_DEN) == 0 || (radioData.status & RB_STATUS_BATT) == BATTERY_VERYLOW){
			dancing = 0;
		} else if(trajectoryUpdate) {
			dep.x = currentX;
			dep.y = currentY;
			dep.angle = currentOrientation*128/M_PI;
			dep.date = getDate();
			trajectoryUpdate = 0;

			currentInterpoints = computeInterpoints(&dep, currentMove);
			dancing = 1; // allow robot to move
		}

		// move only if allowed
		if(dancing) {
			getGoal(getDate(), &dep, currentMove, &currentInterpoints, &dist, &diff);

			distGoal = CM_TO_TICKS*dist;
			angleGoal = CM_TO_TICKS*diff;
		}

		// compute absolute position
		updatePosition(&currentOrientation);

		chThdSleepMilliseconds(1000/GOALS_REFRESH_RATE);
	}
}

void initMotion(void) {
	chThdCreateStatic(waMotion, sizeof(waMotion), NORMALPRIO-1, motionThread, NULL);
}
