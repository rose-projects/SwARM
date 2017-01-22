#include "hal.h"

#include "usbcfg.h"
#include "chprintf.h"
#include "coding_wheels.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"


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

    // Starting test with enslavement
    chprintf(COUT, "Starting wheels' tests with motors\r\n");
    start_asservs();
    chThdSleepMilliseconds(3000);
    start_moving();

    // Looping on a sleep to test the enslavement
    while(true)
        chThdSleepMilliseconds(100);

    return 0;
}
