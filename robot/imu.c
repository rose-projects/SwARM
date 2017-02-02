#include <string.h>
#include "ch.h"
#include "hal.h"
#include "math.h"

#include "RTT/SEGGER_RTT.h"
#include "my_i2c.h"
#include "MPU9250.h"
#include "dance.h"
#include "led.h"
#include "../shared/flash.h"

#define MAG_RES 10.0*4219.0/32760.0 // 16 bit mode magnetometer LSB to milliGauss
#define MFS_16BITS 1 << 4           // 16 bits resolution magnetometer
#define MAG_MODE 0x06               // magnetometer at 100Hz

#define CALIBRATION_REPEAT 4 // how many times to repeat calibration to be sure
#define MAGIC_REF 0xDEADBEEF

// RAM buffer to save calibration during page clear
static float magBiasRAM[3], magScaleRAM[3];
static unsigned int magicCodeRAM;

// non volatile data
static float magbias[3] __attribute__((section(".flashdata")));  // mag bias calibration data
static float magscale[3] __attribute__((section(".flashdata"))); // mag scale calibration data
static unsigned int magicCode __attribute__((section(".flashdata")));  // mismatch w/ MAGIC_REF -> need calibration

static float magCalibration[3] = {0, 0, 0}; // Factory mag calibration

volatile float azimuth;

// MPU 9250 & AK 8963 PART

// INIT PART
static void initMPU9250(void) {
	// Write a one to bit 7 reset bit; toggle reset device
	writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x80);
	chThdSleepMilliseconds(100);

	// Initialize MPU9250 device
	// wake up device : Clear sleep mode bit (6), enable all sensors
	writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x00);
	chThdSleepMilliseconds(100);

	// get stable time source
	// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
	writeByte(MPU9250_ADDRESS, PWR_MGMT_1, 0x01);

	// Disable FSYNC
	// DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz
	// Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
	writeByte(MPU9250_ADDRESS, CONFIG, 0x03);

	// Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
	// Use a 200 Hz rate; the same rate set in CONFIG above
	writeByte(MPU9250_ADDRESS, SMPLRT_DIV, 0x04);

	// Configure Interrupts and Bypass Enable
	// Set interrupt pin active high, push-pull,
	// clear on read of INT_STATUS, enable I2C_BYPASS_EN so additional chips
	writeByte(MPU9250_ADDRESS, INT_PIN_CFG, 0x22);
	// Enable data ready (bit 0) interrupt
	writeByte(MPU9250_ADDRESS, INT_ENABLE, 0x01);
}

static void resetAK8963(void) {
	// reset all registers
	writeByte(AK8963_ADDRESS, AK8963_CNTL2, 0x01);
	chThdSleepMilliseconds(100);
}

static void initAK8963(float * calibration) {

	// First extract the factory calibration for each magnetometer axis
	// x/y/z mag calibration data stored here
	uint8_t rawData[3];

	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);
	// Enter Fuse ROM access mode
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x0F);
	chThdSleepMilliseconds(10);

	// Read the x-, y-, and z-axis calibration values
	readBytes(AK8963_ADDRESS, AK8963_ASAX, 3, &rawData[0]);

	// Return x-axis sensitivity adjustment values, etc.
	calibration[0] = (float)(rawData[0] - 128)/256.0f + 1.0f;
	calibration[1] = (float)(rawData[1] - 128)/256.0f + 1.0f;
	calibration[2] = (float)(rawData[2] - 128)/256.0f + 1.0f;

	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);

	// Configure the magnetometer for continuous read at highest resolution
	writeByte(AK8963_ADDRESS, AK8963_CNTL, MFS_16BITS | MAG_MODE);
	chThdSleepMilliseconds(10);
}

// MEASURES PART
static int readMagData(int16_t * data) {
	// x/y/z mag register and ST2 register datas stored here
	// must read ST2 at end of data acquisition
	uint8_t rawData[7];

	// wait for magnetometer data ready bit to be set
	if(readByte(AK8963_ADDRESS, AK8963_ST1) & 0x01) {
		// Read the six raw data and ST2 registers sequentially into data array
		readBytes(AK8963_ADDRESS, AK8963_XOUT_L, 7, &rawData[0]);
		// End data read by reading ST2 register
		uint8_t c = rawData[6];

		// Check if magnetic sensor overflow set, if not then report data
		if(!(c & 0x08)) {
			// Turn the MSB and LSB into a signed 16-bit value : Data stored as little Endian
			data[0] = (int16_t)(((int16_t)rawData[1] << 8) | rawData[0]);
			data[1] = (int16_t)(((int16_t)rawData[3] << 8) | rawData[2]);
			data[2] = (int16_t)(((int16_t)rawData[5] << 8) | rawData[4]);
			return 0;
		} else {
			return 2;
		}
	} else {
		return 1;
	}
}

// CALIBRATION PART
static int mag_self_test(void) {
	int16_t datas[6]; // x/y/z mag register data
	int ret;

	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);
	// Enter SelfTest mode
	writeByte(AK8963_ADDRESS, AK8963_ASTC, 0x40);
	chThdSleepMilliseconds(10);
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x08);
	chThdSleepMilliseconds(10);

	// get self test datas
	ret = readMagData(datas);
	// stop self test mode
	writeByte(AK8963_ADDRESS, AK8963_ASTC, 0x00);
	chThdSleepMilliseconds(10);
	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);

	return ret;
}

// the values will be hardcoded in the future
static void mag_calibration(float * bias, float * scale) {
	int i = 0;
	int j = 0;
	uint16_t sample_count = 1024;
	int32_t mag_bias[3] = {0, 0, 0};
	int32_t mag_scale[3] = {0, 0, 0};
	int16_t mag_max[3] = {0x8000, 0x8000, 0x8000};
	int16_t mag_min[3] = {0x7FFF, 0x7FFF, 0x7FFF};
	int16_t mag_temp[3] = {0, 0, 0};

	for(i = 0; i < sample_count; i++) {
		readMagData(mag_temp);
		for (j = 0; j < 3; j++) {
			if(mag_temp[j] > mag_max[j]) mag_max[j] = mag_temp[j];
			if(mag_temp[j] < mag_min[j]) mag_min[j] = mag_temp[j];
		}

		// at 100 Hz ODR, new mag data is available every 1 ms
		// we use 10 millisecond to complete 8 movements by hand
		chThdSleepMilliseconds(10);
	}

	// Get hard iron correction
	mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
	mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
	mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts

	// save mag biases in G for main program
	bias[0] = (float) mag_bias[0]*MAG_RES*magCalibration[0];
	bias[1] = (float) mag_bias[1]*MAG_RES*magCalibration[1];
	bias[2] = (float) mag_bias[2]*MAG_RES*magCalibration[2];

	// Get soft iron correction estimate
	mag_scale[0]  = (mag_max[0] - mag_min[0])/2;  // get average x axis max chord length in counts
	mag_scale[1]  = (mag_max[1] - mag_min[1])/2;  // get average y axis max chord length in counts
	mag_scale[2]  = (mag_max[2] - mag_min[2])/2;  // get average z axis max chord length in counts

	float avg_rad = mag_scale[0] + mag_scale[1] + mag_scale[2];
	avg_rad /= 3.0;

	scale[0] = avg_rad/((float)mag_scale[0]);
	scale[1] = avg_rad/((float)mag_scale[1]);
	scale[2] = avg_rad/((float)mag_scale[2]);
}

// copy calibration data into RAM buffers
void saveIMUcalibration(void) {
	memcpy(magBiasRAM, magbias, sizeof(magbias));
	memcpy(magScaleRAM, magscale, sizeof(magscale));
	magicCodeRAM = magicCode;
}

// write IMU calibration data in flash
void writeIMUcalibration(void) {
	// write IMU calibration
	flashWrite((flashaddr_t) &magbias, (char*) magBiasRAM, sizeof(magbias));
	flashWrite((flashaddr_t) &magscale, (char*) magScaleRAM, sizeof(magscale));
	// write magic code
	flashWrite((flashaddr_t) &magicCode, (char*) &magicCodeRAM, sizeof(magicCode));
}

static void imu_calibration(void) {
	int i = 0;
	float magBias[3], magScale[3];

	for(i = 0; i < CALIBRATION_REPEAT; i++) {
		printf("Mag Calibration n° %d : Wave device in a figure eight until done!\n", i+1);
		mag_calibration(magBias, magScale);
		printf("Calibration done\n");

		magBiasRAM[0] += magBias[0];
		magBiasRAM[1] += magBias[1];
		magBiasRAM[2] += magBias[2];
		magScaleRAM[0] += magScale[0];
		magScaleRAM[1] += magScale[1];
		magScaleRAM[2] += magScale[2];
	}

	// compute calibration mean
	magBiasRAM[0] /= CALIBRATION_REPEAT;
	magBiasRAM[1] /= CALIBRATION_REPEAT;
	magBiasRAM[2] /= CALIBRATION_REPEAT;
	magScaleRAM[0] /= CALIBRATION_REPEAT;
	magScaleRAM[1] /= CALIBRATION_REPEAT;
	magScaleRAM[2] /= CALIBRATION_REPEAT;

	// save dance before writing flash
	saveDance();
	// erase and write back dance moves
	writeStoredData();
	// write calibration
	magicCodeRAM = MAGIC_REF;
	writeIMUcalibration();
}

// IMU thread
static THD_WORKING_AREA(waIMU, 256);
static THD_FUNCTION(imuThread, th_data) {
	int16_t magOutput[3]; // Stores the 16-bit signed magnetometer sensor output
	float mx, my;         // variables to hold latest sensor data values

	(void) th_data;
	chRegSetThreadName("IMU");

	// actual reading loop
	while(1) {
		// Read the x/y/z adc values
		if(!readMagData(magOutput)) {
			// Include factory calibration per data sheet and user environmental corrections
			// get actual magnetometer valuez in milliGauss, this depends on scale being set
			mx = ((float) magOutput[0])*MAG_RES*magCalibration[0] - magbias[0];
			my = ((float) magOutput[1])*MAG_RES*magCalibration[1] - magbias[1];
			mx *= magscale[0];
			my *= magscale[1];

			// all angles are from the north
			// they will be from the X axis of the scene in the future
			if(mx == 0) {
				if(my < 0) {
					azimuth = M_PI/2;
				} else { // y > 0
					azimuth = - M_PI/2;
				}
			} else if (mx < 0) {
				azimuth = M_PI - atan(my/mx);
			} else if (mx > 0) {
				if (my < 0) {
					azimuth = - atan(my/mx);
				} else { // y > 0
					azimuth = 2 * M_PI - atan(my/mx);
				}
			}
		}

		// at 100 Hz ODR, new mag data is available every 10 ms
		chThdSleepMilliseconds(10);
	}
}

// actual called function
// print all values in loop
int initIMU(void) {

	initI2C();
	// Read the WHO_AM_I register for MPU-9250, this is a good test of communication
	uint8_t whoami = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if (whoami != 0x71) {
		printf("Could not connect to IMU\n");
		setColor(0, 255, 100); // turn LEDs red
		return 3;
	}

	// init IMU
	initMPU9250();

	// Read the WHO_AM_I register for AK-8963, this is a good test of communication
	whoami = readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
	if(whoami != 0x48) {
		printf("Could not connect to AK8963\n");
		setColor(0, 255, 100); // turn LEDs red
		return 2;
	}

	// Initialize device for active mode read of magnetometer
	resetAK8963();

	if(mag_self_test()) {
		printf("self-test fail\n");
		setColor(0, 255, 100); // turn LEDs red
		return 1;
	}

	initAK8963(magCalibration);

	// Calibration of magnetometer : 0 times -> ue preset value
	// User environmental axis correction in milliGauss
	if(magicCode != MAGIC_REF)
		imu_calibration();

	initAK8963(magCalibration);

	// start IMU thread
	chThdCreateStatic(waIMU, sizeof(waIMU), NORMALPRIO, imuThread, NULL);
	return 0;
}
