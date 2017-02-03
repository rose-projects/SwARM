#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "asser.h"
#include "coding_wheels.h"
#include "wheel_constants.h"
#include "pwmdriver.h"
#include "motors.h"
#include "coordination.h"

#define MIN(a,b) ((a>b) ? b : a)
#define MAX(a,b) ((a>b) ? a : b)

// Enslavement thread working area
static THD_WORKING_AREA(working_area_asser_thd, 128);

// Error on the last commands, shared with moving_thread
volatile int dist_error = 0;
volatile int angle_error = 0;

// Errors of the enslavement
static int dist_error_sum;
static int dist_error_delta;
static int dist_error_prev;
static int angle_error_sum;
static int angle_error_delta;
static int angle_error_prev;

// Enslavement calculations
static THD_FUNCTION(asser_thd, arg) {
    (void) arg;

    // ASSER frequency in Hz
    const unsigned int ASSER_FREQ_HZ = 200;
    // ASSER THREADS sleep time in ms
    const unsigned int ASSER_THD_SLEEP = (1000/ASSER_FREQ_HZ);

    // PID coefficients for angle and distance
    const double P_ANGLE = 2;
    const double I_ANGLE = 0.002;
    const double D_ANGLE = 5;
    const double P_DIST = 1.3333333;
    const double I_DIST = 0.002;
    const double D_DIST = 5;

    int cmd_left; // left wheel PWM command
    int cmd_right; // right wheel PWM command
    int cmd_dist; // distance command calculated by enslavement
    int cmd_angle; // angle command calculated by enslavement
    int angle; // current angle
    int distance; // current distance

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

        // Calculating cmd values
        cmd_left = cmd_dist - cmd_angle;
        cmd_right = cmd_dist + cmd_angle;

        /* 
         * Calculating motor commands value
         * Firt we have the basic sum and difference
         * Then we normalize the values so obtained so that they don't exceed
         * PWM_MAX/2 and are above -PWM_MAX/2 and mimic the truncations so that
         * the values are still "proportionate" after normalization
         */

        // Standardizing command values regarding their sign
        int cmd_max = MAX(cmd_right, cmd_left);
        int cmd_min = MIN(cmd_right, cmd_left);
        int offset_pos = 0;
        int offset_neg = 0;

        if(cmd_max > 200){
            offset_pos = cmd_max - PWM_MAX/2;
        }


        if(cmd_min < -200){
            offset_neg = cmd_min + PWM_MAX/2;
        }


        cmd_left = cmd_left - offset_pos - offset_neg;
        cmd_right = cmd_right - offset_pos - offset_neg;

        if(cmd_left >= 0){
            cmd_left = MIN(cmd_left, PWM_MAX/2);
        }
        else{ 
            cmd_left = 0;
        }

        if(cmd_right >= 0){
            cmd_right = MIN(cmd_right, PWM_MAX/2);
        }
        else{
            cmd_right = 0;
        }

        // Updating PWM signals
        setLpwm(cmd_left);
        setRpwm(cmd_right);

        printf("cmd_left %d\r\n", cmd_left);
        printf("cmd_right %d\r\n", cmd_right);
        printf("tick_l %d\r\n", tick_l);
        printf("tick_r %d\r\n", tick_r);

        // Go to sleep
        chThdSleepMilliseconds(ASSER_THD_SLEEP);
    }
}

// To be called from main to start a basic enslavement
void initAsser(){
    // Motors init
    initMotors();

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
