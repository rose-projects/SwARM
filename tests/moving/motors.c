#include "motors.h"
#include "hal.h"
#include "ch.h"

#define PWM_FREQ            (100000)
#define FULL_FORWARD        palSetPad(GPIOF,GPIOF_STAT1);\
                            palSetPad(GPIOC,GPIOC_SWITCH_TAMPER);

                            

const PWMConfig pwm_conf = {
    // Frequency
    PWM_FREQ,
    // Period of the PWM generation
    200,
    // Period callback 
    NULL,
    {
        // Activate channel 1
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        // Activate channel 2
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
    // PWM for left motor
    palSetPadMode(GPIOE, GPIOE_11, PAL_MODE_ALTERNATE(1));
    // PWM for right motor
    palSetPadMode(GPIOE, GPIOE_9, PAL_MODE_ALTERNATE(1));
    // Input for left motor
    palSetPadMode(GPIOF, GPIOF_STAT1, PAL_MODE_OUTPUT_PUSHPULL);
    // Input for right motor
    palSetPadMode(GPIOC, GPIOC_SWITCH_TAMPER, PAL_MODE_OUTPUT_PUSHPULL);

    // Start PWM on the motors to 0, so that the robots don't move
    pwmStart(&PWMD1, &pwm_conf);

    // Setting the rotating direction of the wheels
    FULL_FORWARD

    pwmDisableChannel(&PWMD1,0);
    pwmDisableChannel(&PWMD1,1);
}
