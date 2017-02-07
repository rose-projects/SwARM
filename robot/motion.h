#ifndef MOTION_H
#define MOTION_H

/* start motion thread, controlling robot moves */
void initMotion(void);

/* set the robot at the origin of the dance */
void resetPosition(void);

/* recompute the trajectory from current position to currentMove (see dance.*) */
void updateInterpoints(void);

#endif
