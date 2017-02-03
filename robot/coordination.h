#ifndef COORDINATION_H
#define COORDINATION_H

void updatemaincoordinates(int x_goal, int y_goal, float arrival_angle,
                           int r_dep, int r_goal, int n_pts);
void updatesubcoordinates(void);

extern volatile int last_angle_error;
extern volatile int last_dist_error;

#endif // COORDINATION_H

