#include "ch.h"
#include "hal.h"
#include "chthreads.h"
#include "shell.h"

#include "usbconf.h"
#include "exticonf.h"
#include "led.h"
#include "radiocomms.h"
#include "robot.h"
#include "battery.h"

static THD_WORKING_AREA(waShell, 1024);

static const ShellCommand shCmds[] = {
	{"mbcal",   mbCalibrate},
	{"sb1cal",   sb1Calibrate},
	{"sb2cal",   sb2Calibrate},
	{"setid",   setDeviceUID},
	{"getid",   getDeviceUID},
	{"list", dumpConnectedDevices},
	{"beacon", setBeaconPosition},
	{NULL, NULL}
};

static const ShellConfig shConfig = {
	(BaseSequentialStream *) &SDU1,
	shCmds
};


int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();
	shellInit();
	// initialize hardware
	initExti();
	initUSB();
	initLEDs();

	// start radio thread
	startRadio();

	while(1) {
		if (SDU1.config->usbp->state == USB_ACTIVE) {
			shellCreateStatic(&shConfig, waShell, 512, NORMALPRIO);
		}
		chThdSleepMilliseconds(1000);
	}
}
