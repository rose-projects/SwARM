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

    // Let the system have a little nap to let us start picocom 
    chThdSleepMilliseconds(5000);

    chprintf(COUT, "Starting wheels' tests with motors");
    // Starting enslavement tests on both wheels
    start_asservs();

    // Let the system have a little nap to let us start picocom 
    chThdSleepMilliseconds(5000);

    // Looping on a print of the speed value
    for(int i =0; i<2000; i++){
        chprintf(COUT, "tick_l: %D\r\n", tick_l);
        chprintf(COUT, "tick_r: %D\r\n", tick_r);
        chThdSleepMilliseconds(50);
    }

    return 0;
}
