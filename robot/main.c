#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "adcconf.h"
#include "pwmdriver.h"
#include "asser.h"
#include "moving.h"
#include "coding_wheels.h"
#include "swarmShell.h"
#include "led.h"
#include "radiocomms.h"
#include "dance.h"
#include "imu.h"

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
	initLEDs();
	initAsser();
	initSequencer();
	//initIMU();
	//initSwarmShell();

    angle_goal = 0;
    dist_goal = 0;

	chThdSleepMilliseconds(2000);

	// start radio thread
	startRadio();
	printf("Ah oui oui oui oui oui !\n");

	while(1)
		chThdSleepMilliseconds(1000);
}
