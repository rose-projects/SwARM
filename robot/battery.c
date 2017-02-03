#include "ch.h"
#include "hal.h"
#include "radiocomms.h"
#include "../shared/radioconf.h"
#include "led.h"

// sample to battery voltage (in 0.01V) conversion coeff
#define PROBE_TO_VBAT 450/4096

#define SAMPLES_HISTORY 16
static adcsample_t sample;

// battery transition thresholds
#define BATTERY_HIGH_LTHRES 380

#define BATTERY_OK_HTHRES 390
#define BATTERY_OK_LTHRES 340

#define BATTERY_LOW_HTHRES 350
#define BATTERY_LOW_LTHRES 300

#define BATTERY_VERYLOW_HTHRES 310

int batteryState = BATTERY_OK;

static void adcErrorCallback(ADCDriver *adcp, adcerror_t err) {
	(void)adcp;
	(void)err;
}

// conversion of one sample of channel 1
static const ADCConversionGroup adcconf = {
	FALSE,                   // linear buffer (not circular)
	1,                       // one channel
	NULL,                    // end of conversion callback
	adcErrorCallback,        // error callback
	0,                       // CFGR
	ADC_TR(0, 4095),         // TR1
	{                        // SMPR[2]
		ADC_SMPR1_SMP1_1 | ADC_SMPR1_SMP1_2, // sample time = 181.5 ADC clk cycles
		0
	},
	{                        // SQR[4]
		ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
		0,
		0,
		0
	}
};

static void updateState(int voltage) {
	switch (batteryState) {
	case BATTERY_HIGH:
		if(voltage < BATTERY_HIGH_LTHRES)
			batteryState = BATTERY_OK;
		break;
	case BATTERY_OK:
		if(voltage > BATTERY_OK_HTHRES)
			batteryState = BATTERY_HIGH;
		if(voltage < BATTERY_OK_LTHRES)
			batteryState = BATTERY_LOW;
		break;
	case BATTERY_LOW:
		if(voltage > BATTERY_LOW_HTHRES)
			batteryState = BATTERY_OK;
		if(voltage < BATTERY_LOW_LTHRES)
			batteryState = BATTERY_VERYLOW;
		break;
	case BATTERY_VERYLOW:
		if(voltage > BATTERY_VERYLOW_HTHRES)
			batteryState = BATTERY_LOW;
	}

	// set battery state flag in status
	chSysLock(); // lock to guarantee atomicity
	radioData.status &= ~RB_STATUS_BATT;
	radioData.status |= batteryState;
	chSysUnlock();

	// TODO: stop the robot when battery is VERY_LOW
	if(batteryState == BATTERY_VERYLOW)
		setColor(0, 255, 40);
}

static THD_WORKING_AREA(waBattery, 128);
static THD_FUNCTION(batteryThread, th_data) {
	int i = 0, voltage = 0;

	(void) th_data;
	chRegSetThreadName("Battery");

	while(1) {
		// start a conversion
		adcConvert(&ADCD1, &adcconf, &sample, 1);
		voltage += sample;

		// compute voltage when SAMPLES_HISTORY samples has been collected
		if(++i == SAMPLES_HISTORY) {
			updateState(voltage/SAMPLES_HISTORY);
			i = 0;
			voltage = 0;
		}

		chThdSleepMilliseconds(800);
	}
}

void initBattery(void) {
	// power up ADC1
	adcStart(&ADCD1, NULL);

	// start battery probe thread
	chThdCreateStatic(waBattery, sizeof(waBattery), NORMALPRIO-2, batteryThread, NULL);
}
