#include "moving.h"
#include "coding_wheels.h"
#include "asser.h"
#include "position.h"
#include "coordination.h"

#include "hal.h"
#include "ch.h"
#include "usbcfg.h"
#include "chprintf.h"

// We update robot position goal every 50 milliseconds
#define MOVING_THD_SLEEP 50

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 128);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
    (void) arg;
    int i = 0;
    int n_pts = 42; // number of intermediary points, sets the arrival date

    /* 
     * Thread routine
     * it takes care of the trajectory  control:
     * it modifies on a periodic basis the  dist_goal and angle_goal values
     * so that the robot moves to the next position
     */
    while(i<n_pts){
        if((i%n_pts) == 0){
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "Begin of new cycle\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            // Resetting enslavement error variables
            begin_new_asser();
        }
        // Updating position, dist/angle error offset to add to next commands
        update_position();
        // Calculate next target position and update distance and angle goals

        // Ready for next iteration
        i++;
        chprintf(COUT, "##########################\r\n");
        chprintf(COUT, "Updating distance and angle goals\r\n");
        chprintf(COUT, "Distance new value: %D\r\n", dist_goal);
        chprintf(COUT, "Angle new value: %D\r\n", angle_goal);

        // Go to sleep
        chThdSleepMilliseconds(MOVING_THD_SLEEP);
    }
}

// To be called from main to start the enslavement with some distance and goal
void start_moving(){
    // Starting the monitoring threads
    (void)chThdCreateStatic(working_area_moving_thd, \
            sizeof(working_area_moving_thd),
            NORMALPRIO, moving_thd, NULL);
}
