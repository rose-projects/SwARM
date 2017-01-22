#include "hal.h"
#include "ch.h"

#include "moving.h"
#include "coding_wheels.h"
#include "asser.h"
#include "position.h"
#include "coordination.h"

// We update robot position goal every 200 milliseconds
#define MOVING_THD_SLEEP 50

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 128);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
    (void) arg;
    int i = 0;

    /* 
     * Thread routine
     * it takes care of the trajectory  control:
     * it modifies on a periodic basis the  dist_goal and angle_goal values
     * so that the robot moves to the next position
     */
    while(i<N_POINTS){
        if((i%N_POINTS) == 0){
            update_main_coordinates(0,0,0);
        }
        // Updating position, dist/angle error offset to add to next commands
        update_position();
        // Calculate next target position and update distance and angle goals
        update_sub_coordinates();
        // Resetting enslavement error variables
        begin_new_asser();

        // Ready for next iteration
        i++;

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
