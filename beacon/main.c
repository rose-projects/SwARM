#include "ch.h"
#include "chvt.h"
#include "chprintf.h"
#include "chevents.h"
#include "chthreads.h"
#include "shell.h"

#include "../shared/usb-config.h"
#include "../shared/exti-config.h"
#include "../shared/non-volatile.h"
#include "radio-comms.h"
#include "robot.h"

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(1024)

static const ShellCommand shCmds[] = {
    {"mbcal",   mbCalibrate},
	{"sb1cal",   sb1Calibrate},
	{"sb2cal",   sb2Calibrate},
	{"setid",   setDeviceUID},
	{"getid",   getDeviceUID},
    {NULL, NULL}
};

static const ShellConfig shConfig = {
    (BaseSequentialStream *) &SDU1,
    shCmds
};

int main(void) {
	static thread_t *sh = NULL;

	// initialize ChibiOS
    halInit();
    chSysInit();
	shellInit();
	// initialize interrupt on decawave IRQ
	initExti();
    // initialize serial over USB
    initUSB();
	// start radio thread
	startRadio();

	while(1) {
		if (!sh && SDU1.config->usbp->state == USB_ACTIVE) {
	    	sh = shellCreate(&shConfig, SHELL_WA_SIZE, NORMALPRIO);
	    } else if (chThdTerminatedX(sh)) {
			chThdRelease(sh);
			sh = NULL;
	    }
		chThdSleepMilliseconds(1000);
	}
}
