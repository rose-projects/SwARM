#include "ch.h"
#include "hal.h"
#include "pid.h"
#include "coding_wheels.h"

// Events sources
EVENTSOURCE_DECL(deca_event);

/* Decawave EXTI callback */
static void decaIRQ_cb(EXTDriver *extp, expchannel_t channel) {
	(void)extp;
	(void)channel;

	chSysLockFromISR();
	chEvtBroadcastFlagsI(&deca_event, EVENT_MASK(0));
	chSysUnlockFromISR();
}

// external interrupts configuration
static const EXTConfig extcfg = {
	{
		{EXT_CH_MODE_DISABLED, NULL}, // 0
		{EXT_CH_MODE_DISABLED, NULL}, // 1
		{EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOD, decaIRQ_cb}, // 2
		{EXT_CH_MODE_DISABLED, NULL}, // 3
		{EXT_CH_MODE_DISABLED, NULL}, // 4
		{EXT_CH_MODE_DISABLED, NULL}, // 5
		{EXT_CH_MODE_DISABLED, NULL}, // 6
		{EXT_CH_MODE_DISABLED, NULL}, // 7
		{EXT_CH_MODE_DISABLED, NULL}, // 8
		{EXT_CH_MODE_DISABLED, NULL}, // 9
		{EXT_CH_MODE_DISABLED, NULL}, // 10
		{EXT_CH_MODE_DISABLED, NULL}, // 11
		{EXT_CH_MODE_DISABLED, NULL}, // 12
		{EXT_CH_MODE_DISABLED, NULL}, // 13
		{EXT_CH_MODE_DISABLED, NULL}, // 14
		{EXT_CH_MODE_DISABLED, NULL}, // 15
		{EXT_CH_MODE_DISABLED, NULL}, // 16 : PVD
		{EXT_CH_MODE_DISABLED, NULL}, // 17 : RTC alarm
		{EXT_CH_MODE_DISABLED, NULL}, // 18 : USB wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 19 : RTC tamper
		{EXT_CH_MODE_DISABLED, NULL}, // 20 : RTC wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 21 : COMP1 output
		{EXT_CH_MODE_DISABLED, NULL}, // 22 : COMP2 output
		{EXT_CH_MODE_DISABLED, NULL}, // 23 : I2C1 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 24 : I2C2 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 25 : USART1 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 26 : USART2 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 27 : I2C3 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 28 : USART3 wakeup
		{EXT_CH_MODE_DISABLED, NULL}, // 29 : reserved
		{EXT_CH_MODE_DISABLED, NULL}  // 30 : COMP4 output
	}
};

void initExti(void) {
	extStart(&EXTD1, &extcfg);

	extChannelEnableI(&EXTD1, 2);
}
