#ifndef LED_H
#define LED_H

void initSPI(void);
void turn_off_leds(void);
void rainbow(int delay, uint8_t intensity);

#endif