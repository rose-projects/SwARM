#include "ch.h"

#include "../shared/usb-config.h"
#include "../shared/exti-config.h"
#include "radio-comms.h"

int main(void) {
	// initialize ChibiOS
    halInit();
    chSysInit();
	// initialize interrupt on decawave IRQ
	initExti();
    // initialize serial over USB
    initUSB();
	// start radio thread
	startRadio();

	while(1)
		chThdSleepMilliseconds(10000);
}
