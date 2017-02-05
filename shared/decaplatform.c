/* deca-plateform.c :
* This implements all the platform specific functions required by the Decawave
* driver
*/

#include "ch.h"
#include "hal.h"
#include "decadriver/deca_device_api.h"

// 2.25MHz SPI configuration, CPHA=0, CPOL=0
static const SPIConfig slowspiconfig = {
	NULL,
	GPIOA,
	GPIOA_DWM_SPI_CSn ,
	SPI_CR1_BR_1 | SPI_CR1_BR_0,
	0
};
// 18MHz SPI configuration, CPHA=0, CPOL=0
static const SPIConfig spiconfig = {
	NULL,
	GPIOA,
	GPIOA_DWM_SPI_CSn,
	0,
	0
};

void initDecaPlatform(void) {
	// make sure chip select is not active
	palSetLine(LINE_DWM_CSn);

	// init SPI3 with SPI clock at 2.6MHz
	spiStart(&SPID3, &slowspiconfig);
}

void useFastSPI(void) {
	spiStart(&SPID3, &spiconfig);
}

int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer) {
	spiSelect(&SPID3);                  // Slave Select assertion.

	spiSend(&SPID3, headerLength, headerBuffer);
	spiSend(&SPID3, bodylength, bodyBuffer);

	spiUnselect(&SPID3);                // Slave Select de-assertion.
	return 0;
}

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer) {
	spiSelect(&SPID3);                  // Slave Select assertion.

	spiSend(&SPID3, headerLength, headerBuffer);
	spiReceive(&SPID3, readlength, readBuffer);

	spiUnselect(&SPID3);                // Slave Select de-assertion.
	return 0;
}

decaIrqStatus_t decamutexon(void) {
	return 0;
}

void decamutexoff(decaIrqStatus_t s) {
	(void) s;
}

void deca_sleep(unsigned int time_ms) {
	chThdSleepMilliseconds(time_ms);
}
