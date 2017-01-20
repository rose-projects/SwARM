#include "ch.h"
#include "hal.h"

// number of LED module to drive, from 1 to 8
#define NB_LED 2
// correction coefficients to get closer to the target color
#define R_COEFF 1
#define G_COEFF 0.8
#define B_COEFF 0.8

// adapted from http://stackoverflow.com/a/14733008
static void hsv2rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b) {
    unsigned int region, rem, p, q, t;

    if (s == 0) {
        *r = v;
        *g = v;
        *b = v;
		return;
    } else {
	    region = h / 43;
	    rem = (h - (region * 43)) * 6;

	    p = v*(255 - s) >> 8;
	    q = v*((255 - (s * rem)) >> 8) >> 8;
	    t = v*(255 - (s * (255 - rem) >> 8)) >> 8;

	    switch (region) {
	    case 0:
	        *r = v; *g = t; *b = p;
	        break;
	    case 1:
	        *r = q; *g = v; *b = p;
	        break;
	    case 2:
	        *r = p; *g = v; *b = t;
	        break;
	    case 3:
	        *r = p; *g = q; *b = v;
	        break;
	    case 4:
	        *r = t; *g = p; *b = v;
	        break;
	    default:
	        *r = v; *g = p; *b = q;
	        break;
	    }
	}
}

/*
 * data  = [start][LED1]...[LEDn][end]
 * start = 0x00 : 32 bits
 * LED   = [brightness : 0xE0(204) + value : 8bits]
 *         [blue : 8bits][green : 8 bits][red : 8 bits]
 * end   = 0xFF : n/2 bits
 * (Note : all data are inverted because of the LED's level shifter)
 */
// set the LEDs at the same color (HSV color space)
static void setLEDs(uint8_t h, uint8_t s, uint8_t v) {
	int i;
	// data to send to SPI
	uint8_t data[4] = {0xFF,0xFF,0xFF,0xFF};

	// send start frame
	spiSend(&SPID2, 4, data);

	// generate data for one LED
	data[0] = 0;
	hsv2rgb(h, s, v, &data[3], &data[2], &data[1]);
	// inverse data to compensate level shifter
	data[1] = ~((uint8_t)(data[1]*B_COEFF));
	data[2] = ~((uint8_t)(data[2]*G_COEFF));
	data[3] = ~((uint8_t)(data[3]*R_COEFF));

	// send it to all the LED modules
	for(i=0; i<NB_LED; i++)
		spiSend(&SPID2, 4, data);

	// send end frame
	data[0] = 0;
	spiSend(&SPID2, 1, data);
}

static uint8_t *hgoal, *sgoal, *vgoal;

// refresh period (in ms) : defines fader steps length
#define REFRESH_PERIOD 5
// fade time (in ms)
#define FADE_TIME 100
// returns 1 if x>0, -1 if x<0 and 0 if x=0
#define SIGN(x) ((x > 0) - (x < 0))

static THD_WORKING_AREA(waFader, 128);
static THD_FUNCTION(faderThread, th_data) {
	uint8_t h = 0, s = 0, v = 0;
	uint8_t htarget = 0, starget = 0, vtarget = 0;
	int hstep = 0, sstep = 0, vstep = 0;

	(void) th_data;
	chRegSetThreadName("Fader");

	while(1) {
		// if goals have changed, update steps
		if(htarget != *hgoal || starget != *sgoal || vtarget != *vgoal) {
			htarget = *hgoal;
			starget = *sgoal;
			vtarget = *vgoal;

			hstep = (htarget - h)/(FADE_TIME/REFRESH_PERIOD) + SIGN(htarget - h);
			sstep = (starget - s)/(FADE_TIME/REFRESH_PERIOD) + SIGN(starget - s);
			vstep = (vtarget - v)/(FADE_TIME/REFRESH_PERIOD) + SIGN(vtarget - v);
		}

		// update LEDs if target is not reached
		if(htarget != h || starget != s || vtarget != v) {
			if((hstep>0 && (h + hstep)>htarget) || (hstep<0 && (h + hstep)<htarget))
				h = htarget;
			else
				h += hstep;

			if((sstep>0 && (s + sstep)>starget) || (sstep<0 && (s + sstep)<starget))
				s = starget;
			else
				s += sstep;

			if((vstep>0 && (v + vstep)>vtarget) || (vstep<0 && (v + vstep)<vtarget))
				v = vtarget;
			else
				v += vstep;

			setLEDs(h, s, v);
		}

		chThdSleepMilliseconds(REFRESH_PERIOD);
	}
}

// SPI2 configuration (562.5kHz, CPHA=0, CPOL=1, MSb first)
static const SPIConfig spiconf = {
	NULL,
	GPIOA,
	GPIOA_PIN8, // CSn redirected to an unused pin
	SPI_CR1_CPOL | SPI_CR1_BR_2 | SPI_CR1_BR_0,
	0
};

// take control of the color locally
void setColor(uint8_t h, uint8_t s, uint8_t v) {
	static uint8_t hlocal, slocal, vlocal;

	hlocal = h;
	slocal = s;
	vlocal = v;
	hgoal = &hlocal;
	sgoal = &slocal;
	vgoal = &vlocal;
}

void releaseColor(void) {
	//TODO
}

// LEDs setup
void initLEDs(void) {
	// Setup transfer paramters
	spiStart(&SPID2, &spiconf);
	// init color goal pointers
	setColor(33, 255, 128);

	// create fader thread
	chThdCreateStatic(waFader, sizeof(waFader), NORMALPRIO-1, faderThread, NULL);
}
