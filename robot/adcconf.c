#include "ch.h"
#include "hal.h"
#include "radiocomms.h"
#include "../shared/radioconf.h"
#include "led.h"
#include "coding_wheels.h"

// sample to battery voltage (in 0.01V) conversion coeff
#define PROBE_TO_VBAT (450/4096)

#define BATTERY_CHANNEL ADC_CHANNEL_IN1
#define RCODER_CHANNEL ADC_CHANNEL_IN11
#define LCODER_CHANNEL ADC_CHANNEL_IN15

#define ADC_CHANNELS 13
static adcsample_t samples[ADC_CHANNELS];

#define SAMPLES_MAX (6*4096)
#define CODER_HTHRES (SAMPLES_MAX * 5/8)
#define CODER_LTHRES (SAMPLES_MAX * 3/8)

// compute the mean over BATTERY_SAMPLES samples
#define BATTERY_SAMPLES 16

// battery transition thresholds
#define BATTERY_HIGH_LTHRES 380

#define BATTERY_OK_HTHRES 390
#define BATTERY_OK_LTHRES 340

#define BATTERY_LOW_HTHRES 350
#define BATTERY_LOW_LTHRES 300

#define BATTERY_VERYLOW_HTHRES 310

static int batteryState = BATTERY_OK;

volatile unsigned int tick_l = 0, tick_r = 0;


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
	chSysLockFromISR(); // lock to guarantee atomicity
//	radioData.status &= ~RB_STATUS_BATT; // TODO: HELP WTF DOES THAT DO THE MACRO DOES NOT EXIST THE UNIVERSE WILL EXPLOOODE!!!
	radioData.status |= batteryState;
	chSysUnlockFromISR();

	// TODO: stop the robot when battery is VERY_LOW
	if(batteryState == BATTERY_VERYLOW)
		setColor(0, 255, 40);
}

static void adcErrorCallback(ADCDriver *adcp, adcerror_t err) {
	(void)adcp;
	(void)err;
}

static void adcConversionCallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
	static int batterySampleCnt = 0;
	static int voltage = 0;
	static int LcoderLevel = 0, RcoderLevel = 0;
	int Lcoder, Rcoder;

	(void) adcp;
	(void) n;

	Lcoder = buffer[0] + buffer[2] + buffer[4] + buffer[6] + buffer[8] + buffer[10];
	Rcoder = buffer[1] + buffer[3] + buffer[5] + buffer[7] + buffer[9] + buffer[11];

	if(!LcoderLevel && Lcoder > CODER_HTHRES) {
		LcoderLevel = 1;
		tick_l++;
	} else if(LcoderLevel && Lcoder < CODER_LTHRES) {
		LcoderLevel = 0;
		tick_l++;
	}

	if(!RcoderLevel && Rcoder > CODER_HTHRES) {
		RcoderLevel = 1;
		tick_r++;
	} else if(RcoderLevel && Rcoder < CODER_LTHRES) {
		RcoderLevel = 0;
		tick_r++;
	}

	voltage += buffer[12];
	if(++batterySampleCnt == BATTERY_SAMPLES) {
		updateState(voltage*PROBE_TO_VBAT/BATTERY_SAMPLES);
		batterySampleCnt = 0;
		voltage = 0;
	}
}

// conversion of one sample of channel 1
static const ADCConversionGroup adcconf = {
	TRUE,                    // linear buffer (not circular)
	ADC_CHANNELS,            // 3 channels
	adcConversionCallback,   // end of conversion callback
	adcErrorCallback,        // error callback
	ADC_CFGR_CONT,           // CFGR: continuous mode
	ADC_TR(0, 4095),         // TR1
	{                        // SMPR[2]
		ADC_SMPR1_SMP_AN1(ADC_SMPR_SMP_61P5),
		ADC_SMPR2_SMP_AN11(ADC_SMPR_SMP_61P5) | ADC_SMPR2_SMP_AN15(ADC_SMPR_SMP_61P5)
	},
	{                        // SQR[4]
		ADC_SQR1_SQ1_N(LCODER_CHANNEL)  | ADC_SQR1_SQ2_N(RCODER_CHANNEL)  |
		ADC_SQR1_SQ3_N(LCODER_CHANNEL)  | ADC_SQR1_SQ4_N(RCODER_CHANNEL),
		ADC_SQR2_SQ5_N(LCODER_CHANNEL)  | ADC_SQR2_SQ6_N(RCODER_CHANNEL)  |
		ADC_SQR2_SQ7_N(LCODER_CHANNEL)  | ADC_SQR2_SQ8_N(RCODER_CHANNEL)  |
		ADC_SQR2_SQ9_N(LCODER_CHANNEL),
		ADC_SQR3_SQ10_N(RCODER_CHANNEL) | ADC_SQR3_SQ11_N(LCODER_CHANNEL) |
		ADC_SQR3_SQ12_N(RCODER_CHANNEL) | ADC_SQR3_SQ13_N(BATTERY_CHANNEL)
	}
};

void initADC(void) {
	// power up ADC1
	adcStart(&ADCD1, NULL);
	adcStartConversion(&ADCD1, &adcconf, samples, 1);
}
