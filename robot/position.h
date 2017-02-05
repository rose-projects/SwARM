#ifndef POSITION_H
#define POSITION_H

#include "trigo.h"

extern volatile int x_pos;
extern volatile int y_pos;

extern float orientation;

void update_position(void);
void startFusion(void);

#endif // POSITION_H
