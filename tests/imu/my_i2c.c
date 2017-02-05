// I2C Driver PART
#include "ch.h"
#include "hal.h"

// Pin definitions
// int intPin = 12;  // These can be changed, 2 and 3 are the Arduinos ext int pins

// We use the I2C driver 2
#define I2C_SDA (0U)
#define I2C_SDC (1U)
#define I2C_INT (3U)

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


void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
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

uint8_t readByte(uint8_t address, uint8_t subAddress) {
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

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {     
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
