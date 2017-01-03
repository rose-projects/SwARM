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
#define P_ANGLE 0.01
#define I_ANGLE 0
#define D_ANGLE 0
#define P_DIST 0.02
#define I_DIST 0
#define D_DIST 0
#define MAX_ERR_DIST 30
#define MAX_ERR_ANGLE 100
#define MIN(a,b) ((a>b) ? b : a)


static THD_WORKING_AREA(working_area_asser_thd, 128);

// Current values for distance and angle since starting the enslavement
int angle = 0;
int distance = 0;

// Asservissement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;
    int dist_goal = 5000;
    int dist_error;
    int dist_error_sum;
    int dist_error_delta;
    int dist_error_prev;
    int angle_goal = 200;
    int angle_error;
    int angle_error_sum;
    int angle_error_delta;
    int angle_error_prev;
    int cmd_dist;
    int cmd_angle;
    int cmd_dist_adjstd;
    int cmd_angle_adjstd;

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

        // Updating PWMs
        cmd_dist_adjstd = MIN(cmd_dist, 50);
        cmd_angle_adjstd = MIN(cmd_angle, 50);

        if(cmd_dist_adjstd > 0 && cmd_dist_adjstd < 30)
            cmd_dist_adjstd = 30;
        if(cmd_angle_adjstd > 0 && cmd_angle_adjstd < 30)
            cmd_angle_adjstd = 30;

        pwmEnableChannel(&PWMD1, 0, cmd_dist_adjstd);
        pwmEnableChannel(&PWMD1, 1, cmd_angle_adjstd);


        // Printing out the current values of ticks and pwm commands
        chprintf(COUT, "tick_l: %D\r\n", tick_l);
        chprintf(COUT, "tick_r: %D\r\n", tick_r);
        chprintf(COUT, "cmd_dist: %D\r\n", cmd_dist_adjstd);
        chprintf(COUT, "cmd_angle: %D\r\n", cmd_angle_adjstd);
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
