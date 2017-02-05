#include "ch.h"
#include "hal.h"

#include "exticonf.h"
#include "moving.h"
#include "pid.h"
#include "pwmdriver.h"
#include "coding_wheels.h"
#include "radiocomms.h"
#include "led.h"
#include "dance.h"
#include "adcconf.h"
#include "position.h"
#include "imu.h"

int main(void) {
	
	// initialize ChibiOS
	halInit();
	chSysInit();

	// initialize hardware
	initPWM();
	initExti();
	initADC();
	initPID();
	initLEDs();
	initIMU();
	initSequencer();
    
	chThdSleepMilliseconds(2000);

	// start choreography
	startRadio();
    start_moving();
	startFusion();

	while(1) {
		chThdSleepMilliseconds(5000);
	}
}
