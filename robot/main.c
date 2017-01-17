#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "led.h"

int main(void) {
	halInit();
	chSysInit();

	initExti();
	initComparators();
	initPWM();
	initLEDs();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

	printf("Ah oui oui oui oui oui !\n");

	while (true)
		chThdSleepMilliseconds(500);
}
