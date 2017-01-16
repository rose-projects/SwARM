#include "ch.h"
#include "hal.h"
#include "usbconf.h"
#include "exticonf.h"

int main(void) {
	halInit();
	chSysInit();
	initExti();
	initUSB();

	while (true)
		chThdSleepMilliseconds(500);
}
