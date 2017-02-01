#ifndef ASSER_H
#define ASSER_H

void initAsser(void);
void begin_new_asser(void);

extern volatile int dist_goal;
extern volatile int angle_goal;
extern volatile int dist_error;
extern volatile int angle_error;

#endif
