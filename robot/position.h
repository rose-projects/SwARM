#ifndef POSITION_H
#define POSITION_H

#ifndef DEBUG_ACH
extern int x_pos;
extern int y_pos;
#else
extern volatile int x_pos;
extern volatile int y_pos;
#endif // DEBUG_ACH

extern float orientation;

void update_position(void);

#endif // POSITION_H
