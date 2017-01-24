#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

#include "asser.h"
#include "coding_wheels.h"
#include "wheel_constants.h"
#include "motors.h"
#include "coordination.h"

// ASSER frequency in Hz
#define ASSER_FREQ 200
// ASSER THREADS sleep time in ms
#define ASSER_THD_SLEEP (1000/ASSER_FREQ)
// PID coefficients for angle and distance
#define P_ANGLE 1.33333333
#define I_ANGLE 0.03333333
#define D_ANGLE 0.8
#define P_DIST 3.33333333
#define I_DIST 0.0001
#define D_DIST 9
#define MIN(a,b) ((a>b) ? b : a)
#define MAX(a,b) ((a>b) ? a : b)
#define MAX_POWER 200

// Enslavement thread working area
static THD_WORKING_AREA(working_area_asser_thd, 128);
volatile int dist_error = 0;
volatile int angle_error = 0;
static int dist_error_sum;
static int dist_error_delta;
static int dist_error_prev;
static int angle_error_sum;
static int angle_error_delta;
static int angle_error_prev;

// Enslavement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;
    int cmd_left;
    int cmd_right;
    int cmd_dist;
    int cmd_angle;
    int angle = 0;
    int distance = 0;

    // 200 Hz calculation
    while(true){
        /*
         * Enslavement PID related calculations:
         * Calculating errors
         * Calculating output
         */

        // Distance and error calculations
        angle = tick_r - tick_l;
        distance = (tick_r + tick_l)/2;

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
        cmd_angle = P_ANGLE*angle_error + I_ANGLE*angle_error_sum \
                    + D_ANGLE*angle_error_delta;

        /*
         * Limiting all cmd values so that they don't exceed the maximum value
         * that the pwmEnableChannel can interpret without ambiguosity
         * The maximmun value they can take is 200 so MAX_POWER should be less 
         * than 200
         * Also if the value is negative, the wheel they control will move in
         * reverse
         */

        // Calculating cmd values
        cmd_left = cmd_dist - cmd_angle;
        cmd_right = cmd_dist + cmd_angle;

        // Standardizing command values regarding their sign
        if(cmd_left < 0){
            cmd_left = 0;
        }
        if(cmd_right < 0){
            cmd_right = 0;
        }

        int cmd_max = MAX(cmd_right, cmd_left);
        int offset = cmd_max - MIN(cmd_max, MAX_POWER);

        cmd_left = cmd_left - offset;
        cmd_right = cmd_right - offset;

        // Standardizing command values regarding their sign
        if(cmd_left < 0){
            cmd_left = 0;
        }
        if(cmd_right < 0){
            cmd_right = 0;
        }

        cmd_left = MIN(cmd_left, MAX_POWER);
        cmd_right = MIN(cmd_right, MAX_POWER);

        // Printing out the current values of ticks and pwm commands 
        if(cmd_dist != 0){
            chprintf(COUT, "tick_l: %D\r\n", tick_l); 
            chprintf(COUT, "tick_r: %D\r\n", tick_r); 
            chprintf(COUT, "dist_goal: %D\r\n", dist_goal); 
            chprintf(COUT, "angle_goal: %D\r\n", angle_goal); 
            chprintf(COUT, "distance: %D\r\n", distance); 
            chprintf(COUT, "angle: %D\r\n", angle); 
            chprintf(COUT, "cmd_dist: %D\r\n", cmd_dist); 
            chprintf(COUT, "cmd_angle: %D\r\n", cmd_angle); 
            chprintf(COUT, "cmd_left: %D\r\n", cmd_left); 
            chprintf(COUT, "cmd_right: %D\r\n", cmd_right); 
            chprintf(COUT, "##########################################\r\n"); 
        }

        // Updating PWM signals
        pwmEnableChannel(&PWMD1, 0, cmd_left);
        pwmEnableChannel(&PWMD1, 1, cmd_right);

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
    // Reset angle and distance error integrals
    last_angle_error = 0;
    last_dist_error = 0;
}
