#ifndef EXTICONF_H
#define EXTICONF_H

#include "ch.h"

// Events sources: sent on decawave IRQ rising edge
extern event_source_t deca_event;

// enable external interrupts
void initExti(void);

#endif // EXTICONF_H
