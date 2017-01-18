#include "coding_wheels.h"
#include "wheel_constants.h"
#include "hal.h"
#include "usbcfg.h"

/*
 * TIM5_CH1 is connected to the right coding wheel
 * TIM3_TH2 is connected to the left coding wheel
 */

// Wheels ticks init
volatile int tick_l = 0;
volatile int tick_r = 0;

// Coding wheel interrupt on each period of the coding wheel feedback
// It restarts the "watchdog" that resets the speed
// It calculates the speed
// And it updates the position
static void period_cb_l(ICUDriver * icup){
    (void) icup;
    chSysLockFromISR();
    // Update tick count on the wheel
    tick_l ++;
    chSysUnlockFromISR();
}

static void period_cb_r(ICUDriver * icup){
    (void) icup;
    chSysLockFromISR();
    // Update tick count on the wheel
    tick_r ++;
    chSysUnlockFromISR();
}

// Input capture configuration for LEFT WHEEL
const ICUConfig icu_conf_l = {
    // We trigger on positive edges
    ICU_INPUT_ACTIVE_HIGH,
    // The frequency at which we check the input's value
    FREQUENCY,
    // Callback function on the positive edges
    NULL,
    // Callback function on a period 
    &period_cb_l,
    // Callback function on a overflow, ie if we missed a input
    NULL,
    // Input capture channel we listen on
    ICU_CHANNEL_2,
    // Dier register of the timer, we reset it and configure it in the function
    // that starts the input capture
    0,
};

// Input capture configuration for RIGHT WHEEL
const ICUConfig icu_conf_r = {
    // We trigger on positive edges
    ICU_INPUT_ACTIVE_HIGH,
    // The frequency at which we check the input's value
    FREQUENCY,
    // Callback function on the positive edges
    NULL,
    // Callback function on a period 
    &period_cb_r,
    // Callback function on a overflow, ie if we missed a input
    NULL,
    // Input capture channel we listen on
    ICU_CHANNEL_1,
    // Dier register of the timer, we reset it and configure it in the function
    // that starts the input capture
    0,
};

// Function to start the input capture, to be called in main
void coding_wheels_start(){

    /*
     * Setting GPIOA0 on TIM5_CH1 for the right coding wheel
     * Setting GPIOB5 on TIM3_CH2 for the left coding wheel
     */
    palSetPadMode(GPIOA, GPIOA_BUTTON_WKUP, PAL_MODE_ALTERNATE(2));
    palSetPadMode(GPIOB, GPIOB_I2S3_SD, PAL_MODE_ALTERNATE(2));

    // Software related init
    icuInit();
    icuStart(&ICUD3, &icu_conf_l);
    icuStart(&ICUD5, &icu_conf_r);
    icuStartCapture(&ICUD3);
    icuStartCapture(&ICUD5);
    icuEnableNotifications(&ICUD3);
    icuEnableNotifications(&ICUD5);
}
