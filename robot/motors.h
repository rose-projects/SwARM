#ifndef MOTORS_H
#define MOTORS_H

/* Invoke if you want the robot to move backward */
#define GO_REVERSE()          palSetLine(LINE_MTR_PHASE_L);\
    palSetLine(LINE_MTR_PHASE_R)
/* Invoke if you want the robot to move forward */
#define GO_FORWARD()          palClearLine(LINE_MTR_PHASE_L);\
    palClearLine(LINE_MTR_PHASE_R)

void initMotors(void);

#endif
