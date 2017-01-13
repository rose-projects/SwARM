#include "ch.h"
#include "hal.h"
#include "usbconf.h"

int main(void) {
	halInit();
	chSysInit();
	initUSB();

	while (true)
		chThdSleepMilliseconds(500);
}
