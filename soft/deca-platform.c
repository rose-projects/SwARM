/* deca-plateform.c :
 * This implements all the platform specific functions required by the Decawave
 * driver
 */
#include "ch.h"
#include "hal.h"
#include "decadriver/deca_device_api.h"

#define DECA_CSn 10 // nCS on PA10
#define DECA_IRQ 0  // IRQ on PA0
#define DECA_CLK 5  // SPI1 CLK on PA5
#define DECA_MISO 6  // SPI1 MISO on PA6
#define DECA_MOSI 5  // SPI1 MOSI on PB5

// 2.6MHz SPI configuration, CPHA=0, CPOL=0
static const SPIConfig spiconfig = {
    NULL,
    GPIOA,
    DECA_CSn,
    SPI_CR1_BR_2
};

void initDecaPlatform(void) {
	// make sure chip select is not active
    palSetPad(GPIOA, DECA_CSn);
	// set up Decawave signals
    palSetPadMode(GPIOA, DECA_IRQ, PAL_MODE_INPUT);
	palSetPadMode(GPIOA, DECA_CLK, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOA, DECA_MISO, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(GPIOB, DECA_MOSI, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOA, DECA_CSn, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);

	// init SPI1 with SPI clock at 2.6MHz
	spiStart(&SPID1, &spiconfig);
}

int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer) {
	spiAcquireBus(&SPID1);              // Acquire ownership of the bus.
	//decamutexon();
    spiSelect(&SPID1);                  // Slave Select assertion.

	spiSend(&SPID1, headerLength, headerBuffer);
	spiSend(&SPID1, bodylength, bodyBuffer);

	spiUnselect(&SPID1);                // Slave Select de-assertion.
	//decamutexoff(0);
    spiReleaseBus(&SPID1);
	return 0;
}

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer) {
	spiAcquireBus(&SPID1);              // Acquire ownership of the bus.
	//decamutexon();
    spiSelect(&SPID1);                  // Slave Select assertion.

	spiSend(&SPID1, headerLength, headerBuffer);
	spiReceive(&SPID1, readlength, readBuffer);

	spiUnselect(&SPID1);                // Slave Select de-assertion.
	//decamutexoff(0);
    spiReleaseBus(&SPID1);
    return 0;
}

decaIrqStatus_t decamutexon(void) {
    //chSysLock();
    return 0;
}

void decamutexoff(decaIrqStatus_t s) {
    (void) s;
	//chSysUnlock();
}

void deca_sleep(unsigned int time_ms) {
    chThdSleepMilliseconds(time_ms);
}
