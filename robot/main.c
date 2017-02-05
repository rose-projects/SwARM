#include "ch.h"
#include "hal.h"
#ifdef DEBUG_ACH
#include "RTT/SEGGER_RTT.h"
#endif // DEBUG_ACH
#include "exticonf.h"
#include "moving.h"
#include "pid.h"
#include "pwmdriver.h"
#include "coding_wheels.h"
#include "adcconf.h"
#include "radiocomms.h"
#include "led.h"
#include "dance.h"
#include "battery.h"

volatile int cmd_left = 0;
volatile int cmd_right = 0;

int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	initPWM();
	initExti();
	initADC();
	initPID();
	initBattery();
	//initLEDs();
	initSequencer();
	startRadio();

	chThdSleepMilliseconds(2000);

	start_moving();
	printf("Started!\n");


	while(1){
		chThdSleepMilliseconds(500);
	}
}

