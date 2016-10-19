// led.c 
// contains SPI exchange functions for SPI1 driver and led driver
// SPI_USE_MUTUAL_EXCLUSION is set to False because we only have one slave

#include "ch.h"
#include "hal.h"

// SET PAL MODE
#define GPIOA_SPI1_SCK 5U
#define GPIOB_SPI1_MOSI 5U
#define ALTERNATE_F 5U

#define NB_LED 8

/*
 * SPI PART
 */

// Maximum speed SPI1 configuration (21MHz, CPHA=0, CPOL=0, MSb first).
static const SPIConfig spicfg = {
	NULL,
	GPIOA,
	4U,
	0
};

// SPI1 I/O pins setup : only CLK and MOSI
void initSPI(void) {
	palSetPadMode(GPIOA, GPIOA_SPI1_SCK, PAL_MODE_ALTERNATE(ALTERNATE_F) | \
	                                     PAL_STM32_OSPEED_HIGHEST); // New SCK.
	palSetPadMode(GPIOB, GPIOB_SPI1_MOSI, PAL_MODE_ALTERNATE(ALTERNATE_F) | \
	                                      PAL_STM32_OSPEED_HIGHEST); // New MOSI.
}

// send datas on SPI1 MOSI
static inline void sendSPI(const uint8_t * datas, const size_t size) {
	spiStart(&SPID1, &spicfg); // Setup transfer parameters.
	spiSend(&SPID1, size, datas); // Send datas
}

/*
 * LED PART
 *
 * data = [start frame][LED1]...[LEDn][end frame]
 * start frame = 0x00 : 32 bits 
 * LED = [brightness : 0xE0 (204) + value : 8bits] 
 *       [blue : 8bits] [green : 8 bits] [red : 8 bits]
 * end frame = 0xFF : n/2 bits
 */

// generate LED color datas : Blue, Cyan, Green, Magenta, Red, Yellow  
static void gen_data(const char color, uint8_t intensity, uint8_t * datas) { 

	intensity += 0xE0;
	datas[0] = intensity;
	datas[1] = 0;
	datas[2] = 0;
	datas[3] = 0;
	switch(color) {
		case 'R':
			datas[3] = 255;
			break;
		case 'G':
			datas[2] = 255;
			break;
		case 'B':
			datas[1] = 255;
			break;
		case 'Y':
			datas[2] = 255;
			datas[3] = 255;
			break;
		case 'C':
			datas[1] = 255;
			datas[2] = 255;
			break;
		case 'M':
			datas[1] = 255;
			datas[3] = 255;
			break;
		// if nothing known we turn off the LEDs
		default:
			break;
	}
}

// set the 8 leds at the same color
void set_leds(const char color, const uint8_t intensity) {
	
	uint8_t i; // counter for loops
	uint8_t datas[4]; // color datas for one led
	uint8_t big_buffer[NB_LED * 4]; // datas to send to led strip
	uint8_t startFrame[4] = {0,0,0,0}; // start frame for led strip
	uint8_t endFrame = 0xff; // end frame for led strip
	
	// get color
	gen_data(color, intensity, datas);

	// can't send datas in loop or the buffer will be re-written
	// generate big buffer of datas to send
	for (i = 0; i < NB_LED * 4; i++) {
		big_buffer[i] = datas[(i+4)%4];
	}

	// send start frame
	sendSPI(startFrame, 4);

	// send LEDs datas
	sendSPI(big_buffer, NB_LED * 4);

	// send end frame
	sendSPI(&endFrame, 1);
}