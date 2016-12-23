#include "asser.h"
#include "coding_wheels.h"
#include "wheel_constants.h"
#include "motors.h"

#include "ch.h"
#include "hal.h"

// ASSER frequency in Hz
#define ASSER_FREQ 200
// ASSER THREADS sleep time in ms
#define ASSER_THD_SLEEP (1000/ASSER_FREQ)
// PID coefficients for angle and distance
#define P_ANGLE 0
#define I_ANGLE 0
#define D_ANGLE 0
#define P_DIST 0
#define I_DIST 0
#define D_DIST 0

static THD_WORKING_AREA(working_area_asser_thd, 128);

// Robot's speed in m/s
volatile int speed_l = 0;
volatile int speed_r = 0;
volatile int speed = 0;
int angle;
int distance;

// Asservissement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;
    static int last_tick_l = 0;
    static int last_tick_r = 0;
    int dist_goal = 0;
    int dist_error;
    int dist_error_sum;
    int dist_error_delta;
    int dist_error_prev;
    int angle_goal = 0;
    int angle_error;
    int angle_error_sum;
    int angle_error_delta;
    int angle_error_prev;
    int right_pwm;
    int left_pwm;
    int cmd_dist;
    int cmd_angle;

    // 200 Hz calculation
    while(true){
        // Update speed
        speed_r = (tick_r - last_tick_r)*U_MM*ASSER_FREQ;
        speed_l = (tick_l - last_tick_l)*U_MM*ASSER_FREQ;

        // Distance and error calculations
        distance = (tick_r + tick_l)/2;
        angle = tick_l - tick_r;

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

        // Update coding wheels' commands
        if(cmd_dist < 100)
            right_pwm = 0;
        else
            right_pwm = 100;
        if(cmd_angle < 100)
            left_pwm = 0;
        else
            left_pwm = 100;

        // Updating PWMs
        pwmEnableChannel(&PWMD1, 0, right_pwm);
        pwmEnableChannel(&PWMD1, 1, left_pwm);

        // Prepare next loop iteration
        last_tick_r = tick_r;
        last_tick_l = tick_l;

        // Go to sleep
        chThdSleepMilliseconds(ASSER_THD_SLEEP);
    }
}

// To be called from main to start a basic enslavement
void start_asservs(){
    // Starting the monitoring threads
    (void)chThdCreateStatic(working_area_asser_thd, sizeof(working_area_asser_thd),
            NORMALPRIO, asser_thd, NULL);
    // Motors init
    motors_init();
}
