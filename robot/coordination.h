#ifndef COORDINATION_H
#define COORDINATION_H

int updatemaincoordinates(void);
void updatesubcoordinates(void);

extern volatile int last_angle_error;
extern volatile int last_dist_error;

#endif // COORDINATION_H

