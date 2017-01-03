#ifndef CODING_WHEELS_H
#define CODING_WHEELS_H

#include <stdint.h>

extern volatile uint16_t tick_l;
extern volatile uint16_t tick_r;

void coding_wheels_start(void);

#endif
