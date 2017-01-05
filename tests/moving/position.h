#ifndef POSITION_H
#define POSITION_H

#include "stdint.h"

extern volatile int x_pos;
extern volatile int y_pos;
extern volatile int orientation;

void update_position(void);

#endif
