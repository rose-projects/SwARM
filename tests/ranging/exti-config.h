#ifndef EXTI_CONFIG_H
#define EXTI_CONFIG_H

#include "ch.h"

// Events sources
extern event_source_t deca_event; // sent on decawave IRQ rising edge

/* enable external interrupts when buttons are pressed */
void initExti(void);

#endif
