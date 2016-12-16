#include "ch.h"
#include "hal.h"
#include "led.h"

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
	
	initSPI();

	turn_off_leds();

	while (true) {
		rainbow(2, 3);
	}
	
	return 0;
}
