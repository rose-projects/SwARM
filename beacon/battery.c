#include "ch.h"
#include "hal.h"
#include "led.h"
#include "../shared/radioconf.h"

// sample to battery voltage (in 0.01V) conversion coeff
#define PROBE_TO_VBAT 450/4096

// battery states and transition thresholds
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

	// display battery state
	switch (batteryState) {
	case BATTERY_HIGH:
		setLEDs(0, 50, 0);
		break;
	case BATTERY_OK:
		setLEDs(30, 10, 0);
		break;
	case BATTERY_LOW:
		setLEDs(50, 5, 0);
		break;
	case BATTERY_VERYLOW:
		setLEDs(30, 0, 0);
	}
}

static int mean_voltage(size_t nsamples) {
	int total = 0;
	for (size_t i = 0; i < nsamples; i++) {
		adcsample_t sample;
		adcConvert(&ADCD1, &adcconf, &sample, 1);
		total += sample;
		// wait between samples to filter out low frequencies of noise
		chThdSleepMilliseconds(800);
	}
	return total * PROBE_TO_VBAT / nsamples;
}

static THD_WORKING_AREA(waBattery, 128);
static THD_FUNCTION(batteryThread, th_data) {
	(void) th_data;
	chRegSetThreadName("Battery");

	while(1) {
		updateState(mean_voltage(16));
	}
}

void initBattery(void) {
	// power up ADC1
	adcStart(&ADCD1, NULL);

	// start battery probe thread
	chThdCreateStatic(waBattery, sizeof(waBattery), NORMALPRIO-2, batteryThread, NULL);
}
