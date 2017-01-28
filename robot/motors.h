#ifndef MOTORS_H
#define MOTORS_H

/* Invoke if you want the robot to move backward */
#define GO_REVERSE()          palSetLine(LINE_MTR_PHASE_L);\
    palClearLine(LINE_MTR_PHASE_R);\
    TIM15->CCER = 0x30;\
    TIM16->CCER = 0x01;
/* Invoke if you want the robot to move forward */
#define GO_FORWARD()          palClearLine(LINE_MTR_PHASE_L);\
    palSetLine(LINE_MTR_PHASE_R);\
    TIM15->CCER = 0x10;\
    TIM16->CCER = 0x03;

void initMotors(void);

#endif
