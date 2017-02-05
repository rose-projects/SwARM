#include "ch.h"
#include "hal.h"

#include "pid.h"
#include "coding_wheels.h"
#include "pwmdriver.h"
#include "coordination.h"

#define MIN(a,b) ((a>b) ? b : a)
#define MAX(a,b) ((a>b) ? a : b)

// Enslavement thread working area
static THD_WORKING_AREA(working_area_pid_thd, 128);

// These counters are used to memorize the ticks' values
volatile unsigned int tick_l_capt = 0;
volatile unsigned int tick_r_capt = 0;

// Errors of the enslavement
static int dist_error_sum;
static int dist_error_delta;
static int dist_error_prev;
static int angle_error_sum;
static int angle_error_delta;
static int angle_error_prev;
static int dist_error = 0;
static int angle_error = 0;

// Enslavement calculations
static THD_FUNCTION(pid_thd, arg) {
	(void) arg;
	const unsigned int PID_FREQ_HZ = 1000;
	const unsigned int PID_THD_SLEEP_MS = (1000/PID_FREQ_HZ);
    const int RAMP = 5;  // Corresponds to maximum delta between two consecutive commands
	
	// PID coefficients for angle and distance
	const double P_ANGLE = 2;
	const double I_ANGLE = 0.002;
	const double D_ANGLE = 40;
	const double P_DIST = 1.33333333;
	const double I_DIST = 0.0004;
	const double D_DIST = 25;

    int cmd_left;
    int cmd_right;
	int cmd_dist;  // distance command calculated by enslavement
	int cmd_angle; // angle command calculated by enslavement
	int angle;     // current angle
	int distance;  // current distance
    
    static int last_cmd_left = 0;
    static int last_cmd_right = 0;

	// 200 Hz calculation
	while(true) {

		// Enslavement PID related calculations:
		// Calculating errors
		// Calculating output

		angle = (tick_r - tick_r_capt) - (tick_l - tick_l_capt);
		distance = ((tick_r - tick_r_capt) + (tick_l - tick_l_capt))/2;

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

		cmd_dist = P_DIST*dist_error + I_DIST*dist_error_sum
				   + D_DIST*dist_error_delta;
		cmd_angle = P_ANGLE*angle_error + I_ANGLE*angle_error_sum
					+ D_ANGLE*angle_error_delta;

		// Calculating motor commands value
		// Firt we have the basic sum and difference 
		// Then we normalize the values so obtained so that they don't
		// exceed PWM_MAX/2 and are above -PWM_MAX/2 and mimic the
		// truncations so that the values are still "proportionate"
		// after normalization 

		cmd_left = cmd_dist - cmd_angle;
		cmd_right = cmd_dist + cmd_angle;

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
		} else{
			cmd_left = 0;
		}

		if(cmd_right >= 0){
			cmd_right = MIN(cmd_right, PWM_MAX/2);
		} else{
			cmd_right = 0;
		}

        if(cmd_left > (last_cmd_left + RAMP)){
            cmd_left = last_cmd_left + RAMP;
        }
        else if (cmd_left < (last_cmd_left - RAMP)){
            cmd_left = MAX(0, last_cmd_left - RAMP);
        }

        if(cmd_right > (last_cmd_right + RAMP)){
            cmd_right = last_cmd_right + RAMP;
        }
        else if (cmd_right < (last_cmd_right - RAMP)){
            cmd_right = MAX(0, last_cmd_right - RAMP);
        }

        last_cmd_left = cmd_left;
        last_cmd_right = cmd_right;
		
		// Updating PWM signals
		setLpwm(cmd_left);
		setRpwm(cmd_right);

		chThdSleepMilliseconds(PID_THD_SLEEP_MS);
	}
}

// To be called from main to start a basic enslavement
void initPID(void) {
	// Starting the monitoring threads
	(void)chThdCreateStatic(working_area_pid_thd,
			sizeof(working_area_pid_thd),
			NORMALPRIO, pid_thd, NULL);
}

// This function resets the variables that are used to enslave the two motors of
// the robot. It is called everytime dist_goal and angle_goal are changed
void begin_new_pid(void) {
	
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
    
    // Memorize tick values
    tick_l_capt = tick_l;
    tick_r_capt = tick_r;
    
    // Move origin of the goals to current situation
    angle_goal = 0;
    dist_goal = 0;
}
