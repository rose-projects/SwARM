#ifndef MOVING_H
#define MOVING_H

#include <stdint.h>

void start_moving(void);
extern volatile int dist_goals[3];
extern volatile int angle_goals[3];
extern volatile int to_the_lefts[3];
extern unsigned int last_tick_cnt_l;
extern unsigned int last_tick_cnt_r;

#endif
