#ifndef CODING_WHEELS_H
#define CODING_WHEELS_H

extern volatile int tick_l;
extern volatile int tick_r;
extern volatile int left_forward;
extern volatile int right_forward;

void coding_wheels_start(void);

#endif
