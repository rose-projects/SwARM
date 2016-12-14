#include "coding_wheels.h"
#include "wheel_constants.h"
#include "hal.h"
#include "usbcfg.h"
#include <math.h>

// Period of the last blink on the coding wheel
volatile float period = 0;
// Robot's speed in m/s
volatile int speed = 0;

// Callback of our house made watchdog that sets the robot's speed to zero 
// after 10ms of no activity on the coding wheel
void vt_tim_cb(void * ptr){
    (void) ptr;
    speed = 0;
}

// Virtual timer that is used in the coding wheel interrupt
static virtual_timer_t wheel_vt;

// Coding wheel interrupt on each period of the coding wheel feedback
// It restarts the "watchdog" that resets the speed
// It calculates the speed
// And it updates the position
void period_cb(ICUDriver * icup){
    chSysLockFromISR();

    // Stop and reset the "watchdog" timer
    chVTResetI(&wheel_vt);

    // Get the period of the last tick on the coding wheel
    period = icuGetPeriodX(icup);
    // Calculate speed
    speed = U_MM * FREQUENCY / period;

    // Restart the "watchdog" timer
    chVTSetI(&wheel_vt, MS2ST(10), vt_tim_cb, NULL);

    chSysUnlockFromISR();
}

// Input capture configuration
ICUConfig icu_conf = {
    // We trigger on positive edges
    ICU_INPUT_ACTIVE_HIGH,
    // The frequency at which we check the input's value
    FREQUENCY,
    // Callback function on the positive edges
    NULL,
    // Callback function on a period 
    &period_cb,
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
    icuInit();
    icuStart(&ICUD5, &icu_conf);
    icuStartCapture(&ICUD5);
    icuEnableNotifications(&ICUD5);
}
