#include "ch.h"
#include "hal.h"

#define LED 13U

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

  // GPIOF pin 6 init as output pushpull
  palSetPadMode(GPIOC, LED, PAL_MODE_OUTPUT_PUSHPULL);
  
  // led1 blinker
  while (true) {
    palSetPad(GPIOC, LED);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC, LED);
    chThdSleepMilliseconds(500);
  }

  return 0;
}
