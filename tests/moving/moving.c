#include "moving.h"
#include "coding_wheels.h"
#include "asser.h"
#include "position.h"
#include "coordination.h"

#include "hal.h"
#include "ch.h"
#include "usbcfg.h"
#include "chprintf.h"

// We update robot position goal every 200 milliseconds
#define MOVING_THD_SLEEP 200

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 128);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
    (void) arg;
    unsigned int last_tick_cnt_l;
    unsigned int last_tick_cnt_r;   
    int i = 0;

    /* 
     * Thread routine
     * it takes care of the trajectory  control:
     * it modifies on a periodic basis the  dist_goal and angle_goal values
     * so that the robot moves to the next position
     */
    while(true){
        last_tick_cnt_l = tick_l;
        last_tick_cnt_r = tick_r;

        // Calling reset function for enslavement 
        begin_new_asser();
        update_position();

        // Printing out the current values of ticks
        chprintf(COUT, "tick_l final value: %D\r\n", last_tick_cnt_l);
        chprintf(COUT, "tick_r final value: %D\r\n", last_tick_cnt_r);

        // Preparation of the next loop iteration
        if((i%N_POINTS) == 0){
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "Begin of new cycle\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            chprintf(COUT, "##########################\r\n");
            update_main_coordinates();
        }
        // Calculate next target position and update distance and angle goals
        update_sub_coordinates();

        // Ready for next iteration
        i++;

        chprintf(COUT, "Updating distance and angle goals\r\n");
        chprintf(COUT, "Distance new value: %D\r\n", dist_goal);
        chprintf(COUT, "Are we going forward?: %D\r\n", forward);
        chprintf(COUT, "Angle new value: %D\r\n", angle_goal);
        chprintf(COUT, "To the left?: %D\r\n", to_the_left);

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