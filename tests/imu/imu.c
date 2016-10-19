#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "terminal.h"

#include "MPU9250.h"
#include "imu.h"

/*
 * Set initial input parameters
 */

enum Mscale_t {
	MFS_14BITS = 0, // 0.6 mG per LSB
	MFS_16BITS      // 0.15 mG per LSB
};

static enum Mscale_t Mscale = MFS_16BITS;   // MFS_14BITS or MFS_16BITS, 14-bit or 16-bit magnetometer resolution
static uint8_t Mmode = 0x06;                // Either 8 Hz 0x02) or 100 Hz (0x06) magnetometer data ODR  
static float mRes;                          // scale resolutions per LSB for the sensors
static int16_t magOutput[3] = {0, 0, 0};    // Stores the 16-bit signed magnetometer sensor output
static float magCalibration[3] = {0, 0, 0}; // Factory mag calibration
static float magbias[3] = {0, 0, 0};        // Factory mag bias
static float magscale[3] = {0, 0, 0};       // Factory mag scale
static float mx, my, mz;                    // variables to hold latest sensor data values 

// Pin definitions
// int intPin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins

/*
 * I2C Driver PART
 */

// We use the I2C driver 2
#define I2C_SDA (0U)
#define I2C_SDC (1U)
#define I2C_INT (3U)

static i2cflags_t errors = 0;

// I2C interface #2
static const I2CConfig i2cfg = {
		OPMODE_I2C,
		400000,
		FAST_DUTY_CYCLE_2,
};

void initI2C(void) {
	palSetPadMode(GPIOF, I2C_SDA, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
	palSetPadMode(GPIOF, I2C_SDC, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
	palSetPadMode(GPIOF, I2C_INT, PAL_MODE_INPUT);

	i2cStart(&I2CD2, &i2cfg);
}


static void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	uint8_t data_write[2];
	data_write[0] = subAddress;
	data_write[1] = data;

	status = i2cMasterTransmitTimeout(&I2CD2, address, data_write, 2, 0, 0, tmo);

	if (status != MSG_OK) {
		i2cGetErrors(&I2CD2);
	}
}

static uint8_t readByte(uint8_t address, uint8_t subAddress) {
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	// store the register data
	uint8_t data[1];
	uint8_t data_write[1];
	data_write[0] = subAddress;

	status = i2cMasterTransmitTimeout(&I2CD2, address, data_write, 1, data, 1, tmo);

	if (status != MSG_OK) {
		return i2cGetErrors(&I2CD2);
	}

	return data[0]; 
}

static void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {     
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	int ii;
	uint8_t data[14];
	uint8_t data_write[1];
	data_write[0] = subAddress;

	status = i2cMasterTransmitTimeout(&I2CD2, address, data_write, 1, data, count, tmo);

	if (status != MSG_OK) {
		i2cGetErrors(&I2CD2);
	}

	for(ii = 0; ii < count; ii++) {
		dest[ii] = data[ii];
	}
}

int ping_ak(void) {
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	static uint8_t txbuf = WHO_AM_I_AK8963;
	static uint8_t rxbuf = 0x00;

	status = i2cMasterTransmitTimeout(&I2CD2, AK8963_ADDRESS, &txbuf, 1, &rxbuf, 1, tmo);

	if (status != MSG_OK) {
		errors = i2cGetErrors(&I2CD2);
		return errors;
	}

	if (rxbuf == 0x48) {
		return 0;
	}
	return -1;
}


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


static void mag_self_test(void) {

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
		chprintf(SERIAL, "can't get slef datas\r\n");
	}
	// stop self test mode
	writeByte(AK8963_ADDRESS, AK8963_ASTC, 0x00);
	chThdSleepMilliseconds(10);
	// Power down magnetometer
	writeByte(AK8963_ADDRESS, AK8963_CNTL, 0x00);
	chThdSleepMilliseconds(10);
}


static void mag_calibration(float * bias, float * scale) {
	uint16_t ii = 0, sample_count = 0;
	int32_t mag_bias[3] = {0, 0, 0}, mag_scale[3] = {0, 0, 0};
	int16_t mag_max[3] = {0x8000, 0x8000, 0x8000}, mag_min[3] = {0x7FFF, 0x7FFF, 0x7FFF}, mag_temp[3] = {0, 0, 0};

	chprintf(SERIAL, "Mag Calibration: Wave device in a figure eight until done!");
	chThdSleepMilliseconds(4000);

	sample_count = 128;
	for(ii = 0; ii < sample_count; ii++) {
		// Read the mag data
		readMagData(mag_temp);
		for (int jj = 0; jj < 3; jj++) {
			if(mag_temp[jj] > mag_max[jj]) mag_max[jj] = mag_temp[jj];
			if(mag_temp[jj] < mag_min[jj]) mag_min[jj] = mag_temp[jj];
		}
		chThdSleepMilliseconds(135);  // at 8 Hz ODR, new mag data is available every 125 ms
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

	chprintf(SERIAL, "Mag Calibration done!");
}


// print all values in loop
void imu(void) {

	// Read the WHO_AM_I register for MPU-9250, this is a good test of communication
	uint8_t whoami = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
	chprintf(SERIAL, "I AM 0x%x\n\r", whoami);
	chprintf(SERIAL, "I SHOULD BE 0x71\n\r");

	// WHO_AM_I should always be 0x68
	if (whoami == 0x71) {
		chprintf(SERIAL, "MPU9250 is online...\n\r");
		initMPU9250();

		whoami = readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);

		if(whoami == 0x48) {
			chprintf(SERIAL, "AK8963 is Online...\n\r");

			// Initialize device for active mode read of magnetometer
			resetAK8963();
			mag_self_test();
			initAK8963(magCalibration);

			chprintf(SERIAL, "AK8963 initialized for active data mode....\n\r"); 
			if(Mscale == 0) chprintf(SERIAL, "Magnetometer resolution = 14  bits\n\r");
			if(Mscale == 1) chprintf(SERIAL, "Magnetometer resolution = 16  bits\n\r");
			if(Mmode == 2) chprintf(SERIAL, "Magnetometer ODR = 8 Hz\n\r");
			if(Mmode == 6) chprintf(SERIAL, "Magnetometer ODR = 100 Hz\n\r");

		} else {
			// Loop forever if communication doesn't happen
			chprintf(SERIAL, "Could not connect to AK8963\n\r");
			chprintf(SERIAL, "%#x \n",  whoami);
			while(1);
		}
	} else {
		// Loop forever if communication doesn't happen
		chprintf(SERIAL, "Could not connect to MPU9250\n\r");
		chprintf(SERIAL, "%#x \n",  whoami);
		while(1);
	}

	// Get magnetometer sensitivity
	getMres();
	chprintf(SERIAL, "Magnetometer sensitivity is %f LSB/G \n\r", 1.0f/mRes);

	// Calibration of magnetometer
	// User environmental axis correction in milliGauss
	mag_calibration(magbias, magscale);
	chprintf(SERIAL, "MagBias :\r\n%f\r\n%f\r\n%f\r\nMagScale :\r\n%f\r\n%f\r\n%f\r\n",\
		magbias[0], magbias[1], magbias[2], magscale[0], magscale[1], magscale[2]);

	// pause to see results
	chprintf(SERIAL, "Let's get some datas\r\n");
	chThdSleepMilliseconds(3000);
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

		chprintf(SERIAL, "%f,%f,%f\r\n", mx, my, mz);

//		chprintf(SERIAL, "mx = %f", mx);
//		chprintf(SERIAL, " my = %f", my);
//		chprintf(SERIAL, " mz = %f  mG\n\r", mz);
		chThdSleepMilliseconds(30);
	}
}