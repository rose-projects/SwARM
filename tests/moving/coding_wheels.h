#ifndef CODING_WHEELS_H
#define CODING_WHEELS_H

extern volatile float period_l;
extern volatile float period_r;
extern volatile int speed_l;
extern volatile int speed_r;
extern volatile int speed;

void coding_wheels_start(void);

#endif
