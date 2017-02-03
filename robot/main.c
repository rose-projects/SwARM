#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "led.h"
#include "battery.h"
#include "radiocomms.h"
#include "imu.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"
#include "position.h"

int main(void) {
	halInit();
	chSysInit();

	initExti();
	initComparators();
    initMotors();
	initLEDs();
	initBattery();
	initIMU();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

	startRadio();
    start_asservs();
    start_moving();
	printf("Ah oui oui oui oui oui !\n");
	startFusion();

	while (true)
		chThdSleepMilliseconds(500);
}
