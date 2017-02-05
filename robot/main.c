#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "led.h"
#include "battery.h"
#include "radiocomms.h"
#include "dance.h"
#include "position.h"
#include "imu.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"

int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	initPWM();
	initExti();
	initComparators();
	initLEDs();
	initBattery();
	initSequencer();
	initIMU();
	startFusion();

	// start radio thread
	startRadio();
	printf("Ah oui oui oui oui oui !\n");

	while(1)
		chThdSleepMilliseconds(1000);
}
