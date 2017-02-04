#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "pwmdriver.h"
#include "asser.h"
#include "moving.h"
#include "led.h"
#include "dance.h"
#include "radiocomms.h"
#include "battery.h"
#include "compdriver.h"

int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	initPWM();
	initComparators();
	initExti();
	initBattery();
	initAsser();
	//initLEDs();
	initSequencer();

	//startRadio();
	start_moving();
	printf("C'est oui !\n");

	while(1)
		chThdSleepMilliseconds(500);
}
