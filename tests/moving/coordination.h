#ifndef COORDINATION_H
#define COORDINATION_H

void update_main_coordinates(int x_goal, int y_goal, double arrival_angle,
                             int r_dep, int r_goal, int n_pts);
void update_sub_coordinates(void);

extern volatile int last_angle_error;
extern volatile int last_dist_error;

#endif // COORDINATION_H

