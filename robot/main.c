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
#include "radiocomms.h"
#include "led.h"
#include "dance.h"
#include "adcconf.h"
#include "imu.h"

volatile int cmd_left = 0;
volatile int cmd_right = 0;

int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
#ifdef DEBUG_ACH
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif // DEBUG_ACH
	initPWM();
	initExti();
	initADC();
	initPID();
	//initLEDs();
	initSequencer();
	initIMU();

	startRadio();

	chThdSleepMilliseconds(2000);

	start_moving();
#ifdef DEBUG_ACH
	printf("Started!\n");
#endif // DEBUG_ACH

	while(1){
		chThdSleepMilliseconds(500);
	}
}
