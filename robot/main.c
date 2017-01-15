#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"

int main(void) {
	halInit();
	chSysInit();
	initExti();
	initComparators();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

	printf("RTT !\n");

	while (true)
		chThdSleepMilliseconds(500);
}
