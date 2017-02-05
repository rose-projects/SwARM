#include "ch.h"
#include "hal.h"

#include "position.h"
#include "pid.h"
#include "imu.h"
#include "radiocomms.h"
#include "coordination.h"
#include "coding_wheels.h"

#define TRUST_DWM 0.8

volatile int x_pos_update;
volatile int y_pos_update;

// Update position according to the coding wheels
// The calculation depends on wether we are rotating to the left or to the right

int absolute(int value) {
	if (value < 0) {
		return -value;
	} else {
		return value;  
	}
}

void update_position(void) {
	static unsigned int tick_l_prev = 0;  // last tick count for left wheel
	static unsigned int tick_r_prev = 0;  // last tick conut for right wheel
	unsigned int tick_l_current = tick_l; // current tick count for left wheel
	unsigned int tick_r_current = tick_r; // current tick count for right wheel

	// Calculating the distance the robot moved in the last 50ms
	int distance_current = (tick_l_current + tick_r_current
	                        - tick_l_prev - tick_r_prev)/2;
	// Calculating the angle the robot turned in the last 50ms
	int angle_current = ((tick_l_current - tick_l_prev)
                             - (tick_r_current - tick_r_prev));

	// Preparing next call of the function
	tick_l_prev = tick_l_current;
	tick_r_prev = tick_r_current;

	// Calculating current angle value in radians
	double angle_rad = angle_current*U_DEGREE_ANGLE*DEG_TO_RAD;

	// Updating orientation
	orientation += angle_rad;

	// angle fusion
	if(absolute(orientation - azimuth) >= 1) {
		orientation = azimuth;
	} else {
		orientation += azimuth;
		orientation /= 2;
	}

	// Calculating last coordinates of the robot
	x_pos += distance_current*mcos(orientation);
	y_pos += distance_current*msin(orientation);

	x_pos_update += distance_current*mcos(orientation);
	y_pos_update += distance_current*msin(orientation);
}

// position and orientation fusion thread
static THD_WORKING_AREA(wa_fusion, 64);
static THD_FUNCTION(fusion_thd, arg) {
	(void) arg;

	int real_x_pos, real_y_pos;
	int old_x_pos, old_y_pos;

	event_listener_t * dwm_update = NULL;
	// register for radio messages datas updates
	chEvtRegisterMask(&radioEvent, dwm_update, EVENT_MASK(0));
	
	while(1) {
		// wait for DWM message
		chEvtWaitAll(EVENT_MASK(0));

		// 0,0 is the special code for no data
		if(radioData.x != 0 && radioData.y != 0) {

			chSysLockFromISR();
			// position fusion : mean with decawave
			real_x_pos = ((TRUST_DWM * radioData.x) + ((1 - TRUST_DWM) * old_x_pos)) / 2;
			real_y_pos = ((TRUST_DWM * radioData.y) + ((1 - TRUST_DWM) * old_y_pos)) / 2;
			
			real_x_pos += x_pos_update;
			real_y_pos += y_pos_update;
			
			x_pos_update = 0;
			y_pos_update = 0;

			// backup position for next message
			old_x_pos = x_pos;
			old_y_pos = y_pos;

			// update real position
			x_pos = real_x_pos;
			y_pos = real_y_pos;

			chSysUnlockFromISR();
		}
	}
}

// To be called from main to start the enslavement with some distance and goal
void startFusion(void) {
	// Starting the monitoring threads
	(void)chThdCreateStatic(wa_fusion, sizeof(wa_fusion), NORMALPRIO, fusion_thd, NULL);
}
