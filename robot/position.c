#include "ch.h"
#include "hal.h"
#include <math.h>

#include "RTT/SEGGER_RTT.h"
#include "imu.h"
#include "codingwheels.h"
#include "dance.h"
#include "trigo.h"
#include "radiocomms.h"

#define TRUST_DWM 0.0
#define TRUST_IMU 0.0

volatile float currentX, currentY, xSinceLastRadio, ySinceLastRadio;

int tickLprev = 0; // last tick count for left wheel
int tickRprev = 0; // last tick count for right wheel

// Update position according to the coding wheels and the IMU
// The calculation depends on wether we are rotating to the left or to the right

void updatePosition(float *currentOrientation) {
	int tickLcurrent = tickL; // snapshot of the current tick count for left wheel
	int tickRcurrent = tickR; // snapshot of the current tick count for right wheel

	// Calculating the distance the robot moved in the last 50ms
	int deltaDistance = (tickLcurrent - tickLprev + tickRcurrent - tickRprev)/2;
	// Calculating the angle the robot turned in the last 50ms
	int deltaAngle = (tickRcurrent - tickRprev) - (tickLcurrent - tickLprev);

	// Preparing next call of the function
	tickLprev = tickLcurrent;
	tickRprev = tickRcurrent;

	// Updating orientation
	*currentOrientation += deltaAngle*TICKS_TO_RAD;
	if(*currentOrientation < 0) {
		*currentOrientation += 2 * M_PI;
	} else if (*currentOrientation >= 2 * M_PI) {
		*currentOrientation -= 2 * M_PI;
	}

	// Angle fusion: IMU & coding wheels
	*currentOrientation = (TRUST_IMU * getAzimuth()) + ((1 - TRUST_IMU) * (*currentOrientation));

	// Calculating last coordinates of the robot
	currentX += deltaDistance*mcos(*currentOrientation)*TICKS_TO_CM;
	currentY += deltaDistance*msin(*currentOrientation)*TICKS_TO_CM;
}

// position and orientation fusion thread
static THD_WORKING_AREA(wa_fusion, 256);
static THD_FUNCTION(fusion_thd, arg) {
	(void) arg;

	float oldX, oldY, diffX, diffY;
	int X, Y;
	// register for radio messages datas updates
	event_listener_t dwm_update;
	chEvtRegisterMask(&radioEvent, &dwm_update, EVENT_MASK(0));
	
	while(1) {

		// wait for DWM message
		chEvtWaitAll(EVENT_MASK(0));

		// 0,0 is the special code for no data
		if(radioData.x != 0 && radioData.y != 0) {

			chSysLock();

			// calculate movement since last radio message
			diffX = currentX - oldX;
			diffY = currentY - oldY;

			// position fusion : mean with decawave
			currentX = (((TRUST_DWM * radioData.x) + ((1 - TRUST_DWM) * oldX))) + diffX;
			currentY = (((TRUST_DWM * radioData.y) + ((1 - TRUST_DWM) * oldY))) + diffY;
			
			// backup position for next message
			oldX = currentX;
			oldY = currentY;

			chSysUnlock();

			// debug print
			X = currentX * 100;
			Y = currentY * 100;

			printf("%d,%d\n", X, Y);
		}
	}
}

// To be called from main to start the enslavement with some distance and goal
void initFusion(void) {
	// Starting the monitoring threads
	(void)chThdCreateStatic(wa_fusion, sizeof(wa_fusion), NORMALPRIO, fusion_thd, NULL);
}
