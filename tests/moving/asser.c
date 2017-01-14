#include "asser.h"
#include "coding_wheels.h"
#include "wheel_constants.h"
#include "motors.h"

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

// ASSER frequency in Hz
#define ASSER_FREQ 200
// ASSER THREADS sleep time in ms
#define ASSER_THD_SLEEP (1000/ASSER_FREQ)
// PID coefficients for angle and distance
#define P_ANGLE 3
#define I_ANGLE 0
#define D_ANGLE 0
#define P_DIST 2
#define I_DIST 0
#define D_DIST 0
#define MIN(a,b) ((a>b) ? b : a)
#define MAX_POWER 200

// Enslavement thread working area
static THD_WORKING_AREA(working_area_asser_thd, 128);
static int dist_error;
static int dist_error_sum;
static int dist_error_delta;
static int dist_error_prev;
static int angle_error;
static int angle_error_sum;
static int angle_error_delta;
static int angle_error_prev;
volatile unsigned int cmd_dist;
volatile unsigned int cmd_angle;
volatile int forward = 1;
volatile int to_the_left = 1;

// Enslavement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;

    // 200 Hz calculation
    while(true){
        // Distance and error calculations
        angle = tick_r - tick_l;
        distance = (tick_r + tick_l)/2;
        if(forward == 1){
            GO_FORWARD
        }
        else{
            GO_REVERSE
        }

        // Error calculations for distance
        dist_error = dist_goal - distance;
        dist_error_sum += dist_error;
        dist_error_delta = dist_error - dist_error_prev;
        dist_error_prev = dist_error;
        // Error calculations for angle
        angle_error = angle_goal - angle;
        angle_error_sum += angle_error;
        angle_error_delta = angle_error - angle_error_prev;
        angle_error_prev = angle_error;

        cmd_dist = P_DIST*dist_error + I_DIST*dist_error_sum \
                   + D_DIST*dist_error_delta;
        cmd_angle = P_ANGLE*angle + I_ANGLE*angle_error_sum \
                    + D_ANGLE*angle_error_delta;

        /*
         * Limiting all cmd values so that they don't exceed the maximum value
         * that the pwmEnableChannel can interpret without ambiguiti
         * The maximmun value they can take is 200 so MAX_POWER should be at
         * less than 200
         */
        cmd_dist = MIN(cmd_dist, MAX_POWER);
        cmd_angle = MIN(cmd_angle, MAX_POWER);

        // Updating PWM signals
        pwmEnableChannel(&PWMD1, 0, cmd_dist);
        pwmEnableChannel(&PWMD1, 1, cmd_angle);

        // Go to sleep
        chThdSleepMilliseconds(ASSER_THD_SLEEP);
    }
}

// To be called from main to start a basic enslavement
void start_asservs(){
    // Motors init
    motors_init();

    // Starting the monitoring threads
    (void)chThdCreateStatic(working_area_asser_thd, \
            sizeof(working_area_asser_thd),
            NORMALPRIO, asser_thd, NULL);
}

/*
 * This functions reset the variables that are used to enslave the two motors o
 * the robot
 * It is called everytime dist_goal and angle_goal are changed
 */
void begin_new_asser(){
    // Reset angle related variables
    angle_error = 0;
    angle_error_sum = 0;
    angle_error_delta = 0;
    angle_error_delta = 0;
    // Reset distance related variables
    dist_error = 0;
    dist_error_sum = 0;
    dist_error_delta = 0;
    dist_error_delta = 0;
    // Motor commands reset
    cmd_dist = 0;
    cmd_angle = 0;
    // Reset ticks
    tick_l = 0;
    tick_r = 0;
}