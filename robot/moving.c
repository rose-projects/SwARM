#include "hal.h"
#include "ch.h"

#include "moving.h"
#include "coding_wheels.h"
#include "pid.h"
#include "position.h"
#include "coordination.h"
#include "dance.h"

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 512);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
	(void) arg;
	
	// We update robot position goal every 50 milliseconds
	const int UPDATE_GOAL_MS = 50;
	const int UPDATE_TRAJ = 20; // * 50ms = 1s
	int pt = 0, npts = 1;
    
    x_pos = 0;
    y_pos = 0;
    orientation = 0;

	// Thread routine
	// it takes care of the trajectory  control:
	// it modifies on a periodic basis the  dist_goal and angle_goal values
	// so that the robot moves to the next position

	for (pt = 0; pt < npts; pt++) {

		if (pt % UPDATE_TRAJ == 0) {
			npts = compute_traj();
		}

		update_position();

		// update distance and angle goals
		if (npts - pt > ADVANCE_TIME) {
			update_goal();
		}

		chThdSleepMilliseconds(UPDATE_GOAL_MS);
	}
}

// To be called from main to start the enslavement with some distance and goal
void start_moving(void) {
	// Starting the monitoring threads
	chThdCreateStatic(working_area_moving_thd, sizeof(working_area_moving_thd), NORMALPRIO, moving_thd, NULL);
}
