#include "moving.h"
#include "asser.h"
#include "coding_wheels.h"

#include "hal.h"
#include "ch.h"
#include "usbcfg.h"
#include "chprintf.h"

// We update robot position goal every 20 seconds
#define MOVING_THD_SLEEP 20000

// Moving control thread working area
static THD_WORKING_AREA(working_area_moving_thd, 128);

// Enslavement calculations
static THD_FUNCTION(moving_thd, arg) {
    (void) arg;

    int i = 0;
    uint16_t current_tick_l;
    uint16_t current_tick_r;

    /* 
     * Thread routine
     * it takes care of the trajectory  control:
     * it modifies on a periodic basis the  dist_goal and angle_goal values
     * so that the robot moves to the next position
     */
    while(true){
        current_tick_l = tick_l;
        current_tick_r = tick_r;
        tick_l = 0;
        tick_r = 0;

        // Printing out the current values of ticks
        chprintf(COUT, "tick_l final value: %D\r\n", current_tick_l);
        chprintf(COUT, "tick_r final value: %D\r\n", current_tick_r);

        chprintf(COUT, "Updating distance and angle goals\r\n");
        dist_goal = dist_goals[i%3];
        angle_goal = angle_goals[i%3];

        // Preparation of the next loop iteration
        i++;
        if(i%3 == 0)
            chprintf(COUT, "Begin of new cycle\r\n");

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
