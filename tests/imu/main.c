#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "imu.h"
#include "terminal.h"

#include "MPU9250.h"

#define LED1 (13U)

// Green LED blinker thread, times are in milliseconds.
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

	(void)arg;
	chRegSetThreadName("blinker");
	
	// led1 blinker
	while (true) {
		palSetPad(GPIOC, LED1);
		chThdSleepMilliseconds(500);
		palClearPad(GPIOC, LED1);
		chThdSleepMilliseconds(500);
	}
}


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

	// GPIOF pin 6 init as output pushpull for green led blinker
	palSetPadMode(GPIOC, LED1, PAL_MODE_OUTPUT_PUSHPULL);
	
	// Creates the blinker thread.
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	// Serial manager initialization
	initSerial();

	// I2C and MPU initialization
	initI2C();

	// let us the time to connect to serial
	chThdSleepMilliseconds(4000);
	chprintf(SERIAL, "terminal start \r\n");

	imu();

	return 0;
}