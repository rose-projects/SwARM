#ifndef LED_H
#define LED_H

/* setup hardware and start fader to control LEDs */
void initLEDs(void);

/* set the color of the LEDs */
void setLEDs(uint8_t r, uint8_t g, uint8_t b);

#endif
