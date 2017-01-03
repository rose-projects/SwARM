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
#define P_ANGLE 4
#define I_ANGLE 0
#define D_ANGLE 0
#define P_DIST 0.025
#define I_DIST 0
#define D_DIST 0
#define MIN(a,b) ((a>b) ? b : a)
#define MAX_POWER 50


static THD_WORKING_AREA(working_area_asser_thd, 128);

// Current values for distance and angle since starting the enslavement
int angle = 0;
int distance = 0;

// Enslavement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;
    int dist_goal = 5000;
    int dist_error;
    int dist_error_sum;
    int dist_error_delta;
    int dist_error_prev;
    int angle_goal = 10;
    int angle_error;
    int angle_error_sum;
    int angle_error_delta;
    int angle_error_prev;
    uint16_t cmd_dist;
    uint16_t cmd_angle;

    // 200 Hz calculation
    while(true){
        // Distance and error calculations
        distance = (tick_r + tick_l)/2;
        angle = tick_r - tick_l;

        // Error calculations
        // For distance
        dist_error = dist_goal - distance;
        dist_error_sum +=dist_error;
        dist_error_delta = dist_error - dist_error_prev;
        dist_error_prev = dist_error;
        // For angle
        angle_error = angle_goal - angle;
        angle_error_sum += angle_error;
        angle_error_delta = angle_error - angle_error_prev;
        angle_error_prev = angle_error;

        cmd_dist = P_DIST*dist_error + I_DIST*dist_error_sum \
                   + D_DIST*dist_error_delta;
        cmd_angle = P_ANGLE*angle + I_ANGLE*angle_error_sum \
                    + D_ANGLE*angle_error_delta;

        /* Adding an offset to all strictly positive cmd_dist values so that
         * the robot moves it needs to
         */
        if(cmd_dist > 0)
            cmd_dist += 35;

        /* Limiting all cmd values so that they don't exceed the maximum value
         * that the pwmEnableChannel can interpret without ambiguiti
         */
        cmd_dist = MIN(cmd_dist, MAX_POWER);
        cmd_angle = MIN(cmd_angle, MAX_POWER);

        /* Translating a negative value on the cmd_angle calculation to maximum
        * power on the right wheel
        if(cmd_angle == -1)
            cmd_angle = MAX_POWER;
        */

        // Updating PWM signals
        pwmEnableChannel(&PWMD1, 0, cmd_dist);
        pwmEnableChannel(&PWMD1, 1, cmd_angle);

        // Printing out the current values of ticks and pwm commands
        chprintf(COUT, "tick_l: %D\r\n", tick_l);
        chprintf(COUT, "tick_r: %D\r\n", tick_r);
        chprintf(COUT, "cmd_dist_adjstd: %D\r\n", cmd_dist);
        chprintf(COUT, "cmd_angle_adjsd: %D\r\n", cmd_angle);
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
