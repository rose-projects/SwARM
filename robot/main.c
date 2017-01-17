#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "radiocomms.h"

int main(void) {
	halInit();
	chSysInit();

	initExti();
	initComparators();
	initPWM();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

	startRadio();
	printf("Ah oui oui oui oui oui !\n");

	while (true)
		chThdSleepMilliseconds(500);
}
