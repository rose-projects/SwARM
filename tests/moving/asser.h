#ifndef ASSER_H
#define ASSER_H

#include <stdint.h>

void start_asservs(void);
extern volatile uint16_t angle;
extern volatile uint16_t distance;
extern volatile uint16_t dist_goal;
extern volatile uint16_t angle_goal;


#endif
