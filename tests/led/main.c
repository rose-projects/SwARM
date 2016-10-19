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

	// GPIOF pin 6 init as output pushpull for led
	palSetPadMode(GPIOC, LED, PAL_MODE_OUTPUT_PUSHPULL);
	
	// set leds
	initSPI();

	// start color for led strip
	char color = 'B';

	// led1 blinker
	while (true) {

		// set leds with 10 of intensity
		set_leds(color, 10);

		// blinking red led on the board
		palSetPad(GPIOC, LED);
		chThdSleepMilliseconds(500);
		palClearPad(GPIOC, LED);
		chThdSleepMilliseconds(500);

		// changing color every second in order : 
		// Blue, Cyan, Green, Magenta, Red, Yellow
		switch(color) {
			case 'B':
				color = 'Y';
				break;
			case 'C':
				color = 'B';
				break;
			case 'G':
				color = 'C';
				break;
			case 'M':
				color = 'G';
				break;
			case 'R':
				color = 'M';
				break;
			case 'Y':
				color = 'R';
				break;
		}
	}

	return 0;
}
