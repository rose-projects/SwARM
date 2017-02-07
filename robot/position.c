#include "ch.h"
#include "hal.h"
#include <math.h>

#include "codingwheels.h"
#include "dance.h"
#include "trigo.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the right
 */
void updatePosition(float *currentX, float *currentY, float *currentOrientation) {
	static int tickLprev = 0; // last tick count for left wheel
	static int tickRprev = 0; // last tick conut for right wheel
	int tickLcurrent = tickL; // snapshot of the current tick count for left wheel
	int tickRcurrent = tickR; // snapshot of the current tick count for right wheel

	// Calculating the distance the robot moved in the last 50ms
	int deltaDistance = (tickLcurrent - tickLprev + tickRcurrent - tickRprev)/2;
	// Calculating the angle the robot turned in the last 50ms
	int deltaAngle = (tickLcurrent - tickLprev) - (tickRcurrent - tickRprev);

	// Preparing next call of the function
	tickLprev = tickLcurrent;
	tickRprev = tickRcurrent;

	// Updating orientation
	*currentOrientation += deltaAngle*TICKS_TO_RAD;
	// Calculating last coordinates of the robot
	*currentX += -deltaDistance*mcos(*currentOrientation)*TICKS_TO_CM;
	*currentY += deltaDistance*msin(*currentOrientation)*TICKS_TO_CM;
}
