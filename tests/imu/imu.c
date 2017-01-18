#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "terminal.h"

#include "math.h"
#include "my_i2c.h"
#include "MPU9250.h"

/*
 * Set initial input parameters
 */

#define PI 3.14159265359
#define NB_CALIB 10 // > 0

enum Mscale_t {
	MFS_14BITS = 0, // 0.6 mG per LSB
	MFS_16BITS      // 0.15 mG per LSB
};

static float mRes;                          // scale resolutions per LSB for the sensors
static float magCalibration[3] = {0, 0, 0}; // Factory mag calibration
static float magbias[3] = {0, 0, 0};        // Factory mag bias
static float magscale[3] = {0, 0, 0};       // Factory mag scale

static enum Mscale_t Mscale = MFS_16BITS;   // MFS_14BITS or MFS_16BITS, 14-bit or 16-bit magnetometer resolution
static uint8_t Mmode = 0x06;                // Either 8 Hz 0x02) or 100 Hz (0x06) magnetometer data ODR  
static int16_t magOutput[3] = {0, 0, 0};    // Stores the 16-bit signed magnetometer sensor output
static float mx, my, mz;                    // variables to hold latest sensor data values 
static float azimuth;

/*
 * MPU 9250 & AK 8963 PART
 */

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
	writeByte(AK8963_ADDRESS, AK8963_CNTL, Mscale << 4 | Mmode);
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
void mag_self_test(void) {

	// x/y/z mag register data
	int16_t datas[6];

	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);
	// Enter SelfTest mode
	writeByte(AK8963_ADDRESS, AK8963_ASTC, 0x40);
	chThdSleepMilliseconds(10);
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x08);
	chThdSleepMilliseconds(10);

	// get self test datas 
	if(!readMagData(datas)) {
		chprintf(SERIAL, "self-test datas : x: %d y: %d y: %d \r\n", datas[0], datas[1], datas[2]);
	} else {
		chprintf(SERIAL, "can't get self-test datas\r\n");
	}
	// stop self test mode
	writeByte(AK8963_ADDRESS, AK8963_ASTC, 0x00);
	chThdSleepMilliseconds(10);
	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);
}

// get magnetometer resolution
static void getMres(void) {
	switch (Mscale) {
	// Possible magnetometer scales (and their register bit settings) are:
	// 14 bit resolution (0) and 16 bit resolution (1)
	case MFS_14BITS:
		mRes = 10.0*4219.0/8190.0;  // Proper scale to return milliGauss
		break;
	case MFS_16BITS:
		mRes = 10.0*4219.0/32760.0; // Proper scale to return milliGauss
		break;
	}
}

// the values will be hardcoded in the future
static void mag_calibration(float * bias, float * scale) {
	int i = 0;
	int j = 0;
	uint16_t sample_count = 2048;
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
		chThdSleepMilliseconds(5);
	}

	// Get hard iron correction
	mag_bias[0]  = (mag_max[0] + mag_min[0])/2;  // get average x mag bias in counts
	mag_bias[1]  = (mag_max[1] + mag_min[1])/2;  // get average y mag bias in counts
	mag_bias[2]  = (mag_max[2] + mag_min[2])/2;  // get average z mag bias in counts

	// save mag biases in G for main program
	bias[0] = (float) mag_bias[0]*mRes*magCalibration[0];
	bias[1] = (float) mag_bias[1]*mRes*magCalibration[1];
	bias[2] = (float) mag_bias[2]*mRes*magCalibration[2];

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

static void imu_calibration(int times) {
	int i = 0;
	float magBias[3];
	float magScale[3];
	
	for(i = 0; i < times; i++) {
		chprintf(SERIAL, "Mag Calibration n° %d : Wave device in a figure eight until done!\r\n", i);
		mag_calibration(magBias, magScale);
		chprintf(SERIAL, "calibration datas : \r\n Mag Bias: %f, %f, %f,\r\n Mag Scale %f, %f, %f,\r\n\r\n",\
			magBias[0],magBias[1],magBias[2],magScale[0],magScale[1],magScale[2]);

		// sum for mean
		magbias[0] += magBias[0];
		magbias[1] += magBias[1];
		magbias[2] += magBias[2];
		magscale[0] += magScale[0];
		magscale[1] += magScale[1];
		magscale[2] += magScale[2];
	}

	// set global calibration mean
	magbias[0] /= times;
	magbias[1] /= times;
	magbias[2] /= times;
	magscale[0] /= times;
	magscale[1] /= times;
	magscale[2] /= times;
}

// actual called function
// print all values in loop
void imu_init(void) {

	// Read the WHO_AM_I register for MPU-9250, this is a good test of communication
	uint8_t whoami = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	if (whoami == 0x71) {
		chprintf(SERIAL, "MPU9250 is online...\n\r");
		initMPU9250();

		// Read the WHO_AM_I register for AK-8963, this is a good test of communication
		whoami = readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
		if(whoami == 0x48) {

			// Initialize device for active mode read of magnetometer
			chprintf(SERIAL, "AK8963 is Online...\n\r");
			resetAK8963();
			mag_self_test();
			initAK8963(magCalibration);

			chprintf(SERIAL, "AK8963 initialized for active data mode....\n\r"); 
			if(Mscale == 0) chprintf(SERIAL, "Magnetometer resolution = 14  bits\n\r");
			if(Mscale == 1) chprintf(SERIAL, "Magnetometer resolution = 16  bits\n\r");
			if(Mmode == 2) chprintf(SERIAL, "Magnetometer ODR = 8 Hz\n\r");
			if(Mmode == 6) chprintf(SERIAL, "Magnetometer ODR = 100 Hz\n\r");

			// Get magnetometer sensitivity
			getMres();
			chprintf(SERIAL, "Magnetometer sensitivity is %f LSB/G \n\r", 1.0f/mRes);

			// Calibration of magnetometer : 10 times
			// User environmental axis correction in milliGauss
			imu_calibration(NB_CALIB);
			chprintf(SERIAL, " MagBias :\r\n%f\r\n%f\r\n%f\r\n MagScale :\r\n%f\r\n%f\r\n%f\r\n", \
				magbias[0], magbias[1], magbias[2], magscale[0], magscale[1], magscale[2]);

			initAK8963(magCalibration);

			// pause to see results
			chThdSleepMilliseconds(3000);

		} else {
			// Loop forever if communication doesn't happen
			chprintf(SERIAL, "Could not connect to AK8963\n\r");
			chprintf(SERIAL, "%#x \n",  whoami);
			while(1);
			// TO DO : Send message to turn LEDs red
		}
	} else {
		// Loop forever if communication doesn't happen
		chprintf(SERIAL, "Could not connect to MPU9250\n\r");
		chprintf(SERIAL, "%#x \n",  whoami);
		while(1);
		// TO DO : Send message to turn LEDs red
	}
}

void imu(void) {
	chprintf(SERIAL, "Let's get some datas\r\n");
	chprintf(SERIAL, "x,y,z");
	
	// actual reading loop
	while(1) {
		// Read the x/y/z adc values
		readMagData(magOutput);
		
		// Calculate the magnetometer values in milliGauss
		// Include factory calibration per data sheet and user environmental corrections

		// get actual magnetometer value, this depends on scale being set
		mx = (float)magOutput[0]*mRes*magCalibration[0] - magbias[0];
		my = (float)magOutput[1]*mRes*magCalibration[1] - magbias[1];
		mz = (float)magOutput[2]*mRes*magCalibration[2] - magbias[2];	
		mx *= magscale[0]; 
		my *= magscale[1]; 
		mz *= magscale[2]; 

		// all angles ar from the north
		// they will be from the X axis of the scene in the future
		if(mx == 0) {
			if(my < 0) {
				azimuth = PI/2;
			} else { // y > 0
				azimuth = - PI/2;
			}
		} else if (mx < 0) {
			azimuth = PI - atan(my/mx); 
		} else if (mx > 0) {
			if (my < 0) {
				azimuth = - atan(my/mx);
			} else { // y > 0
				azimuth = 2 * PI - atan(my/mx);
			}
		}

		chprintf(SERIAL, "Azimuth : %f\r\n", azimuth);
		chThdSleepMilliseconds(20);
	}
}
