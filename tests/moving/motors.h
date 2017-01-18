#ifndef MOTORS_H
#define MOTORS_H

#define GO_REVERSE          palSetPad(GPIOF,GPIOF_STAT1);\
    palSetPad(GPIOC,GPIOC_SWITCH_TAMPER);
#define GO_FORWARD          palClearPad(GPIOF,GPIOF_STAT1);\
    palClearPad(GPIOC,GPIOC_SWITCH_TAMPER);

void motors_init(void);

#endif
