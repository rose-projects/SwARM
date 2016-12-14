#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "chprintf.h"
#include "coding_wheels.h"

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

    /*
     * Initializes a serial-over-USB CDC driver.
     */
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

    /*
     * Sets the GPIOA0 pin to its TIM2..5 alternate function so that we can 
     * capture the timer
     */
    palSetPadMode(GPIOA, GPIOA_BUTTON_WKUP, PAL_MODE_ALTERNATE(2));

    // Start the coding_wheels surveillance
    coding_wheels_start();

    chThdSleepMilliseconds(2000);

    // Looping on a print of the speed value
    while (true) {
        chprintf(COUT, "speed: %D\r\n", speed);
        chThdSleepMilliseconds(50);
    }

    return 0;
}
