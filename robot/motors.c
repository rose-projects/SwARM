#include "hal.h"
#include "ch.h"
#include "coding_wheels.h"
#include "motors.h"
#include "pwmdriver.h"

void initMotors(){
	// Initializing the PWMs controlling the motors
	initPWM();

    // Setting both motors so that they dont move
    setLpwm(0);
    setRpwm(0);
}
