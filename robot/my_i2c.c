// I2C Driver PART
#include "ch.h"
#include "hal.h"

// 0 for fast mode ; 1 for fast mode plus
#define I2C_MODE 0

// I2C interface 1

// Fast Mode
static const I2CConfig i2cfg = {
  STM32_TIMINGR_PRESC(0U) |
  STM32_TIMINGR_SCLDEL(14U) | STM32_TIMINGR_SDADEL(0U) |
  STM32_TIMINGR_SCLH(37U)  | STM32_TIMINGR_SCLL(122U),
  0,
  0
};

// Fast Mode PLus
static const I2CConfig i2cfg_plus = {
  STM32_TIMINGR_PRESC(0U)  |
  STM32_TIMINGR_SCLDEL(10U) | STM32_TIMINGR_SDADEL(0U) |
  STM32_TIMINGR_SCLH(13U)   | STM32_TIMINGR_SCLL(146U),
  0,
  0
};


// already set in board.h : useless ?
void initI2C(void) {
	palSetLineMode(LINE_MPU_I2C_SDA, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
	palSetLineMode(LINE_MPU_I2C_SCL, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
	palSetLineMode(LINE_MPU_INT,     PAL_MODE_INPUT);

	if(I2C_MODE == 0) {
		i2cStart(&I2CD1, &i2cfg);
	} else if (I2C_MODE == 1) {
		i2cStart(&I2CD1, &i2cfg_plus);
	}
}

void writeByte(uint8_t address, uint8_t subAddress, uint8_t data) {
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	uint8_t data_write[2];
	data_write[0] = subAddress;
	data_write[1] = data;

	status = i2cMasterTransmitTimeout(&I2CD1, address, data_write, 2, 0, 0, tmo);

	if (status != MSG_OK) {
		i2cGetErrors(&I2CD1);
	}
}

uint8_t readByte(uint8_t address, uint8_t subAddress) {
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	// store the register data
	uint8_t data[1];
	uint8_t data_write[1];
	data_write[0] = subAddress;

	status = i2cMasterTransmitTimeout(&I2CD1, address, data_write, 1, data, 1, tmo);

	if (status != MSG_OK) {
		return i2cGetErrors(&I2CD1);
	}

	return data[0]; 
}

void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * dest) {     
	msg_t status = MSG_OK;
	systime_t tmo = MS2ST(100);

	int i;
	uint8_t data[14];
	uint8_t data_write[1];
	data_write[0] = subAddress;

	status = i2cMasterTransmitTimeout(&I2CD1, address, data_write, 1, data, count, tmo);

	if (status != MSG_OK) {
		i2cGetErrors(&I2CD1);
	}

	for(i = 0; i < count; i++) {
		dest[i] = data[i];
	}
}
