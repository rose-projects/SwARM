#ifndef MOTORS_H
#define MOTORS_H

#define GO_REVERSE          palSetPad(GPIOF,GPIOF_STAT1);\
    palSetPad(GPIOC,GPIOC_SWITCH_TAMPER);\
    left_forward = -1;\
    right_forward = -1;
#define GO_FORWARD          palClearPad(GPIOF,GPIOF_STAT1);\
    palClearPad(GPIOC,GPIOC_SWITCH_TAMPER);\
    left_forward = 1;\
    right_forward = 1;
#define LEFT_FORWARD        palClearPad(GPIOF, GPIOF_STAT2);\
    left_forward = 1;
#define LEFT_REVERSE        palSetPad(GPIOF, GPIOF_STAT1);\
    left_forward = -1;
#define RIGHT_FORWARD       palClearPad(GPIOC, GPIOC_SWITCH_TAMPER);\
    right_forward = 1;
#define RIGHT_REVERSE       palSetPad(GPIOC, GPIOC_SWITCH_TAMPER);\
    right_forward = -1;

void motors_init(void);

#endif
