#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "chprintf.h"
#include "coding_wheels.h"
#include "motors.h"
#include "asser.h"

// Application entry point.
int main(void) {

    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    //  Initializes a serial-over-USB CDC driver.
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    /*
     * Activates the USB driver and then the USB bus pull-up on D+.
     * Note, a delay is inserted in order to not have to disconnect the cable
     * after a reset.
     */
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Start the coding_wheels surveillance
    coding_wheels_start();

    // Start the motors
    motors_init();

    // Let the system have a little nap to let us start picocom 
    chThdSleepMilliseconds(5000);

    chprintf(COUT, "Starting wheels' tests with motors");
    // Starting with half the power on the motors
    pwmEnableChannel(&PWMD1, 0, 100);
    pwmEnableChannel(&PWMD1, 1, 100);
    start_asservs();

    // Let the system have a little nap to let us start picocom 
    chThdSleepMilliseconds(5000);

    chprintf(COUT, "Starting wheels' tests with motors with half the power \r\n");

    // Looping on a print of the speed value
    for(int i =0; i<200; i++){
        chprintf(COUT, "speed_l: %D\r\n", speed_l);
        chprintf(COUT, "speed_r: %D\r\n", speed_r);
        chprintf(COUT, "speed: %D\r\n", speed);
        chThdSleepMilliseconds(50);
    }

    // Now moving to full power on both engines
    pwmEnableChannel(&PWMD1, 0, 200);
    pwmEnableChannel(&PWMD1, 1, 200);

    chprintf(COUT, "Starting wheels' tests with motors with full power \r\n");
    // Looping on a print of the speed value
    for(int i =0; i<200; i++){
        chprintf(COUT, "speed_l: %D\r\n", speed_l);
        chprintf(COUT, "speed_r: %D\r\n", speed_r);
        chprintf(COUT, "speed: %D\r\n", speed);
        chThdSleepMilliseconds(50);
    }

    return 0;
}
