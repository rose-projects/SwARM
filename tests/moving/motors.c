#include "motors.h"
#include "coding_wheels.h"
#include "hal.h"
#include "ch.h"


#define PWM_FREQ            100000
#define GO_REVERSE          palSetPad(GPIOF,GPIOF_STAT1);\
                            palSetPad(GPIOC,GPIOC_SWITCH_TAMPER);
#define GO_FORWARD          palClearPad(GPIOF,GPIOF_STAT1);\
                            palClearPad(GPIOC,GPIOC_SWITCH_TAMPER);
#define GO_RIGHT            palSetPad(GPIOF,GPIOF_STAT1);\
                            palClearPad(GPIOC,GPIOC_SWITCH_TAMPER);
#define GO_LEFT             palClearPad(GPIOF,GPIOF_STAT1);\
                            palSetPad(GPIOC,GPIOC_SWITCH_TAMPER);

                            

const PWMConfig pwm_conf = {
    // Frequency
    PWM_FREQ,
    // Period of the PWM generation
    200,
    // Period callback 
    NULL,
    {
        // Activate channel 1 for left motor
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        // Activate channel 2 for right motor
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        // Disable channels 3 and 4
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    // CR2 register
    0,
    // DIER register
    0,
};
void motors_init(){
    // Now setting GPIOs' mode
    
    // PWM for left motor
    palSetPadMode(GPIOE, GPIOE_9, PAL_MODE_ALTERNATE(1));
    // PWM for right motor
    palSetPadMode(GPIOE, GPIOE_11, PAL_MODE_ALTERNATE(1));
    // Input for left motor
    palSetPadMode(GPIOF, GPIOF_STAT1, PAL_MODE_OUTPUT_PUSHPULL);
    // Input for right motor
    palSetPadMode(GPIOC, GPIOC_SWITCH_TAMPER, PAL_MODE_OUTPUT_PUSHPULL);

    // Start PWM on the motors
    pwmStart(&PWMD1, &pwm_conf);

    // Setting the direction of the robot
    GO_FORWARD

    // Disable both channels so that the robot stays still
    pwmDisableChannel(&PWMD1,0);
    pwmDisableChannel(&PWMD1,1);

    // Wait for the wheels to stop
    chThdSleepMilliseconds(200);

    // Start the coding wheels to monitor them
    coding_wheels_start();
}
