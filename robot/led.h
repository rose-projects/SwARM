#ifndef LED_H
#define LED_H

/* setup hardware and start fader to control LEDs */
void initLEDs(void);

/* set the color locally, ignoring orders from radio */
void setColor(uint8_t h, uint8_t s, uint8_t v);

/* display the color order received from the radio */
void releaseColor(void);

#endif
