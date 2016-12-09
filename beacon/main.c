#include "ch.h"
#include "chvt.h"
#include "chprintf.h"
#include "chevents.h"

#include "../shared/usb-config.h"
#include "../shared/exti-config.h"
#include "../shared/deca-functions.h"
#include "../shared/decadriver/deca_device_api.h"
#include "../shared/decadriver/deca_regs.h"
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
