#include "ch.h"
#include "hal.h"

#include "position.h"
#include "imu.h"
#include "radiocomms.h"
#include "wheel_constants.h"
#include "asser.h"
#include "coordination.h"

// Update position according to the coding wheels
// The calculation depends on wether we are rotating to the left or to the right

int absolute(int value) {
	if (value < 0) {
		return -value;
	} else {
		return value;  
	}
}

void update_position(){
	// Calculating current angle value in radians
	double angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

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
	x_pos += distance * cos(orientation);
	y_pos += distance * sin(orientation);
	last_dist_error = dist_error;
	last_angle_error = angle_error;
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

			// position fusion : mean with decawave
			real_x_pos = (radioData.x + old_x_pos) / 2;
			real_y_pos = (radioData.y + old_y_pos) / 2;
			real_x_pos += distance * cos(orientation);
			real_y_pos += distance * sin(orientation);

			// backup position for next message
			old_x_pos = x_pos;
			old_y_pos = y_pos;

			// update real position
			x_pos = real_x_pos;
			y_pos = real_y_pos;
		}
	}
}

// To be called from main to start the enslavement with some distance and goal
void startFusion(void){
	// Starting the monitoring threads
	(void)chThdCreateStatic(wa_fusion, sizeof(wa_fusion), NORMALPRIO, fusion_thd, NULL);
}
