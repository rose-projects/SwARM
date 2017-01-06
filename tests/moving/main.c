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

    distance = 0;
    angle = 0;
    dist_goal = 0;
    angle_goal = 0;

    chThdSleepMilliseconds(5000);
    // Starting test with enslavement
    chprintf(COUT, "Starting wheels' tests with motors\r\n");
    start_asservs();
    start_moving();

    // Looping on a sleep to test the enslavement
    for(int i =0; i<20000; i++){
        chThdSleepMilliseconds(50);
    }

    return 0;
}
