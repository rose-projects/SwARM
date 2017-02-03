#include "hal.h"
#include "ch.h"

#include "moving.h"
#include "coding_wheels.h"
#include "asser.h"
#include "position.h"
#include "coordination.h"

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 128);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
	(void) arg;
	// We update robot position goal every 50 milliseconds
	const int UPDATE_SUB_MS = 50; 
	const int UPDATE_TRAJ = 20; // * 50ms = 1s
	int i = 0, npts = 0;

	/* 
	 * Thread routine
	 * it takes care of the trajectory  control:
	 * it modifies on a periodic basis the  dist_goal and angle_goal values
	 * so that the robot moves to the next position
	 */
	npts = updatemaincoordinates();
	for (i = 0; i < npts; i++) {
		if (i % UPDATE_TRAJ == 0) {
			npts = updatemaincoordinates();
			i = 0;
		}
		// dist/angle error offset to add to next commands
		update_position();
		// update distance and angle goals
		updatesubcoordinates();
		// Resetting enslavement error variables
		begin_new_asser();

		chThdSleepMilliseconds(UPDATE_SUB_MS);
	}
}

// To be called from main to start the enslavement with some distance and goal
void start_moving(){
	// Starting the monitoring threads
	(void)chThdCreateStatic(working_area_moving_thd, \
		sizeof(working_area_moving_thd),
		NORMALPRIO, moving_thd, NULL);
}
