#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"
#include "test_motors.h"
#include "led.h"

int main(void) {
	halInit();
	chSysInit();

	initExti();
	initComparators();
    initMotors();
    initLEDs();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    setRpwm(100);
    setLpwm(100);

	while (true)
		chThdSleepMilliseconds(500);
}
