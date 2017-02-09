#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "pwmdriver.h"
#include "led.h"
#include "adcconf.h"
#include "pid.h"
#include "radiocomms.h"
#include "dance.h"
#include "motion.h"
#include "imu.h"
#include "position.h"

int main(void) {
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	initPWM();
	initExti();
	initADC();
	initLEDs();
 	initIMU();

	// start high level features
	initPID();
	startRadio();
	initSequencer();
	initMotion();
	initFusion();

	chThdSleep(TIME_INFINITE);
	return 0;
}
