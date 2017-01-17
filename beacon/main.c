#include "ch.h"
#include "hal.h"
#include "usbconf.h"
#include "exticonf.h"
#include "led.h"

int main(void) {
	halInit();
	chSysInit();
	initExti();
	initUSB();
	initLEDs();

	setLEDs(255, 0, 0); // display red
	
	while (true)
		chThdSleepMilliseconds(500);
}
