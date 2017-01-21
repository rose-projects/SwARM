#include "hal.h"
#include "ch.h"
#include "coding_wheels.h"
#include "motors.h"
#include "pwmdriver.h"

void initMotors(){
    // Initializing the PWMs controlling the motors
    initPWM();

    // Setting the direction of the robot
    GO_FORWARD

    // Disable both channels so that the robot stays still
    setLpwm(0);
    setRpwm(0);

}
