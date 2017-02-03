#ifndef COORDINATION_H
#define COORDINATION_H

int update_main_coordinates(void);
void update_sub_coordinates(void);

extern volatile int last_angle_error;
extern volatile int last_dist_error;
extern int pt; // current point sub point

#endif // COORDINATION_H

