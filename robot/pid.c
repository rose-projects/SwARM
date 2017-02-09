#include "ch.h"
#include "hal.h"

#include "codingwheels.h"
#include "pwmdriver.h"
#include "position.h"

#define MIN(a,b) ((a>b) ? b : a)
#define MAX(a,b) ((a>b) ? a : b)

volatile int distGoal;
volatile int angleGoal;

static THD_WORKING_AREA(waPID, 128);
static THD_FUNCTION(pidThread, arg) {
	(void) arg;

	const unsigned int PID_FREQ_HZ = 1000;
	const int RAMP = 5;  // maximum delta between two consecutive commands

	// PID coefficients for angle and distance
	const double P_ANGLE = 2;
	const double I_ANGLE = 0.02;
	const double D_ANGLE = 4;
	const double P_DIST = 1.3333333;
	const double I_DIST = 0.004;
	const double D_DIST = 2.5;

	// Errors of the enslavement
	int distError;
	int distErrorSum;
	int distErrorDelta;
	int distErrorPrev;

	int angleError;
	int angleErrorSum;
	int angleErrorDelta;
	int angleErrorPrev;

	int cmdLeft;
	int cmdRight;
	int cmdDist;  // distance command
	int cmdAngle; // angle command
	int angle;     // current angle
	int distance;  // current distance

	static int lastCmdLeft = 0;
	static int lastCmdRight = 0;

	// PID related calculations:
	// Calculating errors
	// Calculating output
	while(true) {
		angle = tickR - tickL;
		distance = (tickR + tickL)/2;

		// Error calculations for distance
		distError = distGoal - distance;
		distErrorSum += distError;
		distErrorDelta = distError - distErrorPrev;
		distErrorPrev = distError;

		// Error calculations for angle
		angleError = angleGoal - angle;
		angleErrorSum += angleError;
		angleErrorDelta = angleError - angleErrorPrev;
		angleErrorPrev = angleError;

		cmdDist = P_DIST*distError + I_DIST*distErrorSum + D_DIST*distErrorDelta;
		cmdAngle = P_ANGLE*angleError + I_ANGLE*angleErrorSum + D_ANGLE*angleErrorDelta;

		// prevent error sums from being negative
		if(distErrorSum < 0)
			distErrorSum = 0;
		if(angleErrorSum < 0)
			angleErrorSum = 0;

		// Calculating motor commands value
		// Firt we have the basic sum and difference
		// Then we normalize the values so obtained so that they don't
		// exceed PWM_MAX/2 and are above -PWM_MAX/2 and mimic the
		// truncations so that the values are still "proportionate"
		// after normalization

		cmdLeft = cmdDist - cmdAngle;
		cmdRight = cmdDist + cmdAngle;

		int cmdMax = MAX(cmdRight, cmdLeft);
		int cmdMin = MIN(cmdRight, cmdLeft);
		int offsetPos = 0;
		int offsetNeg = 0;

		if(cmdMax > PWM_MAX/2){
			offsetPos = cmdMax - PWM_MAX/2;
		}

		if(cmdMin < -PWM_MAX/2){
			offsetNeg = cmdMin + PWM_MAX/2;
		}

		cmdLeft = cmdLeft - offsetPos - offsetNeg;
		cmdRight = cmdRight - offsetPos - offsetNeg;

		if(cmdLeft >= 0){
			cmdLeft = MIN(cmdLeft, PWM_MAX/2);
		} else{
			cmdLeft = 0;
		}

		if(cmdRight >= 0){
			cmdRight = MIN(cmdRight, PWM_MAX/2);
		} else{
			cmdRight = 0;
		}

		// avoid accelerating too quickly by limiting PID output variations
		if(cmdLeft > (lastCmdLeft + RAMP)) {
			cmdLeft = lastCmdLeft + RAMP;
		} else if (cmdLeft < (lastCmdLeft - RAMP)) {
			cmdLeft = MAX(0, lastCmdLeft - RAMP);
		}

		if(cmdRight > (lastCmdRight + RAMP)){
			cmdRight = lastCmdRight + RAMP;
		} else if (cmdRight < (lastCmdRight - RAMP)) {
			cmdRight = MAX(0, lastCmdRight - RAMP);
		}

		lastCmdLeft = cmdLeft;
		lastCmdRight = cmdRight;

		// Updating PWM signals
		setLpwm(cmdLeft);
		setRpwm(cmdRight);

		chThdSleepMilliseconds(1000/PID_FREQ_HZ);
	}
}

// To be called from main to start a basic enslavement
void initPID(void) {
	// Starting the monitoring thread
	chThdCreateStatic(waPID, sizeof(waPID), NORMALPRIO, pidThread, NULL);
}

// This function resets the variables that are used to enslave the two motors of
// the robot.
void beginNewPID(void) {
	// reset ticks
	tickL = 0;
	tickR = 0;
	// reset commands
	angleGoal = 0;
	distGoal = 0;

	tickLprev = 0;
	tickRprev = 0;
}
