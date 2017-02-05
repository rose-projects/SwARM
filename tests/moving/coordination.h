#ifndef COORDINATION_H
#define COORDINATION_H

int compute_traj(void);
void update_goal(void);

extern int last_angle_error;
extern int last_dist_error;
extern int pt; // current point sub point

#endif // COORDINATION_H
