#include "ch.h"
#include "hal.h"

// number of LED module to drive, from 1 to 8
#define NB_LED 2
// correction coefficients to get closer to the target color
#define R_COEFF 1
#define G_COEFF 0.8
#define B_COEFF 0.8

/*
 * data  = [start][LED1]...[LEDn][end]
 * start = 0x00 : 32 bits
 * LED   = [brightness : 0xE0(204) + value : 8bits]
 *         [blue : 8bits][green : 8 bits][red : 8 bits]
 * end   = 0xFF : n/2 bits
 * (Note : all data are inverted because of the LED's level shifter)
 */
// set the LEDs at the same color (HSV color space)
void setLEDs(uint8_t r, uint8_t g, uint8_t b) {
	int i;
	// data to send to SPI
	uint8_t data[4] = {0xFF,0xFF,0xFF,0xFF};

	// send start frame
	spiSend(&SPID2, 4, data);

	// generate data for one LED
	data[0] = 0x00;
	// inverse data to compensate level shifter
	data[1] = ~((uint8_t)(b*B_COEFF));
	data[2] = ~((uint8_t)(g*G_COEFF));
	data[3] = ~((uint8_t)(r*R_COEFF));

	// send it to all the LED modules
	for(i=0; i<NB_LED; i++)
		spiSend(&SPID2, 4, data);

	// send end frame
	data[0] = 0;
	spiSend(&SPID2, 1, data);
}

// SPI2 configuration (562.5kHz, CPHA=0, CPOL=1, MSb first)
static const SPIConfig spiconf = {
	NULL,
	GPIOA,
	GPIOA_PIN8, // CSn redirected to an unused pin
	SPI_CR1_CPOL | SPI_CR1_BR_2 | SPI_CR1_BR_0,
	0
};

// LEDs setup
void initLEDs(void) {
	// Setup transfer paramters
	spiStart(&SPID2, &spiconf);
}
