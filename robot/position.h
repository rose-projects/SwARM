#ifndef POSITION_H
#define POSITION_H

extern volatile int x_pos;
extern volatile int y_pos;
extern volatile double orientation;
extern volatile double real_orientation;

void update_position(void);
void startFusion(void);

#endif
