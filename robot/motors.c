#include "hal.h"
#include "ch.h"
#include "coding_wheels.h"
#include "motors.h"
#include "pwmdriver.h"

void initMotors(){
	// Initializing the PWMs controlling the motors
	initPWM();
}
