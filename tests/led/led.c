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

// set the 8 leds at the same color (BGR)
static void set_leds(uint8_t * color, const uint8_t intensity) {
	
	int i; // counter for loops
	uint8_t datas[4]; // datas for one led 
	uint8_t big_buffer[NB_LED * 4]; // datas to send to led strip
	uint8_t startFrame[4] = {0,0,0,0}; // start frame for led strip
	uint8_t endFrame = 0xff; // end frame for led strip
	
	// generate data for one led
	datas[0] = 0xE0 + intensity;
	datas[1] = color[0];
	datas[2] = color[1];
	datas[3] = color[2];

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

// turn of leds
void turn_off_leds(void) {
	uint8_t datas[] = {0, 0 ,0};
	set_leds(datas, 0);
}

// color : B G R
// display all colors with smooth transitions
void rainbow(int delay, uint8_t intensity) {

	int i; // counter for loops
	uint8_t datas[3];
	intensity += 0xE0;

	// red
	for (i = 0; i < 256; i++) {
		datas[0] = 0;
		datas[1] = 0;
		datas[2] = i;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// green -> yellow
	for (i = 0; i < 256; i++) {
		datas[0] = 0;
		datas[1] = i;
		datas[2] = 255;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// red -> green
	for (i = 255; i >= 0; i--) {
		datas[0] = 0;
		datas[1] = 255;
		datas[2] = i;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// blue -> cyan
	for (i = 0; i < 256; i++) {
		datas[0] = i;
		datas[1] = 255;
		datas[2] = 0;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// green -> blue
	for (i = 255; i >= 0; i--) {
		datas[0] = 255;
		datas[1] = i;
		datas[2] = 0;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// red -> violet / pink
	for (i = 0; i < 256; i++) {
		datas[0] = 255;
		datas[1] = 0;
		datas[2] = i;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// green -> red
	for (i = 255; i >= 0; i--) {
		datas[0] = i;
		datas[1] = 0;
		datas[2] = 255;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
	// red -> null
	for (i = 255; i >= 0; i--) {
		datas[0] = 0;
		datas[1] = 0;
		datas[2] = i;
		set_leds(datas, intensity);
		chThdSleepMilliseconds(delay);
	}
}