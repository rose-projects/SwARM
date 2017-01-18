#ifndef ASSER_H
#define ASSER_H

#include <stdint.h>

void start_asservs(void);
void begin_new_asser(void);
extern volatile int angle;
extern volatile int distance;
extern volatile int dist_goal;
extern volatile int angle_goal;
extern volatile int forward;
extern volatile int to_the_left;
extern int dist_error;
extern int angle_error;

#endif
