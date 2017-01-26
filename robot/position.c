#include "ch.h"
#include "hal.h"

#include "position.h"
#include "imu.h"
#include "radiocomms.h"
#include "wheel_constants.h"
#include "asser.h"
#include "coordination.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the right
 */

void update_position(){
	// Calculating current angle value in radians
	double angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

	// Updating orientation
	orientation += angle_rad;
	// angle fusion
	real_orientation = orientation + azimuth;
	real_orientation /= 2;

	// Calculating last coordinates of the robot
	x_pos += distance * cos(orientation);
	y_pos += distance * sin(orientation);
	last_dist_error = dist_error;
	last_angle_error = angle_error;

	// position fusion : mean with decawave
	x_pos += radioData.x;
	y_pos += radioData.y;
	x_pos /= 2;
	y_pos /= 2;
}

// position and orientation fusion thread
static THD_WORKING_AREA(wa_fusion, 64);
static THD_FUNCTION(fusion_thd, arg) {
	(void) arg;

	event_listener_t * dwm_update;
	// register for radio messages datas updates
	chEvtRegisterMask (&radioEvent, dwm_update, EVENT_MASK(0));

	// 10 tuples [time, pos x, pos y]
	int i = 0;
	systime_t time[10];
	float x_pos_dwm[10];
	float y_pos_dwm[10];
	
	while(1) {
		// wait for DWM message
		chEvtBroadcastFlags(&radioEvent, EVENT_MASK(0));

		time[i] = chVTGetSystemTime();

		x_pos += radioData.x;
		y_pos += radioData.y;
	}
}

// To be called from main to start the enslavement with some distance and goal
void startFusion(void){
	// Starting the monitoring threads
	(void)chThdCreateStatic(wa_fusion, sizeof(wa_fusion), NORMALPRIO, fusion_thd, NULL);
}
