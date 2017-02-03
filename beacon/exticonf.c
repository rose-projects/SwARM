#include "ch.h"
#include "hal.h"

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
	}
};

void initExti(void) {
	extStart(&EXTD1, &extcfg);
	extChannelEnable(&EXTD1, 2);
}
