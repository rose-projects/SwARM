#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "adcconf.h"
#include "pwmdriver.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"
#include "coding_wheels.h"
#include "swarmShell.h"

volatile int cmd_left = 0;
volatile int cmd_right = 0;

int main(void) {
	halInit();
	chSysInit();

	initExti();
    initAsser();
	initADC();
	//initSwarmShell();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	palSetLine(LINE_MTR_LED_R);
	palSetLine(LINE_MTR_LED_L);

    angle_goal = 0;
    dist_goal = 0;

	chThdSleepMilliseconds(2000);

	while (true){
        //swarmShellLife();
		chThdSleepMilliseconds(500);
	}
}
