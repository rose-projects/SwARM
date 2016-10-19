#ifndef LED_H
#define LED_H

void initSPI(void);
void set_leds(const char color, const uint8_t intensity);

#endif