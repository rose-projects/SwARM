#include "hal.h"
#include "ch.h"

#include "moving.h"
#include "coding_wheels.h"
#include "pid.h"
#include "position.h"
#include "coordination.h"
#include "dance.h"

#ifdef DEBUG_ACH
#include "RTT/SEGGER_RTT.h"
int dbori;
#endif // DEBUG_ACH

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 512);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
	(void) arg;
	// We update robot position goal every 50 milliseconds
	const int UPDATE_GOAL_MS = 50;
	const int UPDATE_TRAJ = 20; // * 50ms = 1s
	int pt = 0, npts = 1;

	/*
	 * Thread routine
	 * it takes care of the trajectory  control:
	 * it modifies on a periodic basis the  dist_goal and angle_goal values
	 * so that the robot moves to the next position
	 */
	for (pt = 0; pt < npts; pt++) {
	
#ifdef DEBUG_ACH
		printf("pt: %d\n", pt);
#endif // DEBUG_ACH

		if (pt % UPDATE_TRAJ == 0) {
			npts = compute_traj();
		}
		// dist/angle error offset to add to next commands
		
#ifndef DEBUG_ACH
		update_position();
#else
		printf("x_pos: %d\t", x_pos);
		printf("y_pos: %d\t", y_pos);
		dbori = orientation * 100;
		printf("orientation: %d\n", dbori);
#endif // DEBUG_ACH

		// update distance and angle goals
		if (npts - pt > ADVANCE_TIME) {
			update_goal();
		}

		chThdSleepMilliseconds(UPDATE_GOAL_MS);
	}
}

// To be called from main to start the enslavement with some distance and goal
void start_moving(){

#ifdef DEBUG_ACH
	x_pos = 500;
	y_pos = 300;
	orientation = (3.1415*3)/2;
	dbori = orientation * 100;
	printf("start: x_pos: %d, y_pos: %d, orientation: %d\n",
		x_pos, y_pos, dbori);
#endif // DEBUG_ACH

	// Starting the monitoring threads
	chThdCreateStatic(working_area_moving_thd, sizeof(working_area_moving_thd), NORMALPRIO, moving_thd, NULL);
}
