#ifndef COORDINATION_H
#define COORDINATION_H

// N is the number of points between to target postions the robot must go to
#define N_POINTS 20

void update_main_coordinates(void);
void update_sub_coordinates(void);

extern volatile int xb;
extern volatile int yb;
extern volatile int last_angle_error;
extern volatile int last_dist_error;

#endif
