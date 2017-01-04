#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>

void start_moving(void);
extern volatile uint16_t dist_goals[3];
extern volatile uint16_t angle_goals[3];

#endif
