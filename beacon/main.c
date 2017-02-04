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

static const ShellCommand MBshCmds[] = {
	{"mbcal",   mbCalibrate},
	{"sb1cal",   sb1Calibrate},
	{"sb2cal",   sb2Calibrate},
	{"setid",   setDeviceUID},
	{"getid",   getDeviceUID},
	{"list", dumpConnectedDevices},
	{"beacon", setBeaconPosition},
	{"dance", startDance},
	{"stop", stopDance},
	{"clear", clearStoredData},
	{"moves", storeMoves},
	{"colors", storeColors},
	{"flash", writeStoredData},
	{"robot", dumpRobotData},
	{NULL, NULL}
};

static const ShellCommand SBshCmds[] = {
	{"setid",   setDeviceUID},
	{"getid",   getDeviceUID},
	{NULL, NULL}
};

static ShellConfig shConfig = {
	(BaseSequentialStream *) &SDU1,
	SBshCmds
};

int main(void) {
	thread_t *sh = NULL;

	// initialize ChibiOS
	chSysInit();
	halInit();
	shellInit();

	// initialize hardware
	initExti();
	initLEDs();
	initBattery();
	initUSB();

	// start radio thread
	startRadio();

	// extend command set on master beacon
	if(deviceUID == 0)
		shConfig.sc_commands = MBshCmds;

	// show that the system is started before battery level kicks in
	setLEDs(0, 0, 40);

	while(1) {
		if(!sh && SDU1.config->usbp->state == USB_ACTIVE) {
			sh = shellCreateStatic(&shConfig, waShell, 1024, NORMALPRIO);
		}
		chThdSleepMilliseconds(1000);
	}
}
