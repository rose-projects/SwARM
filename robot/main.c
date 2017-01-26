#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"

int main(void) {
	halInit();
	chSysInit();

	initExti();
	initComparators();
    initMotors();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    start_asservs();
    start_moving();

	while (true)
		chThdSleepMilliseconds(500);
}
