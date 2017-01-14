#ifndef EXTICONF_H
#define EXTICONF_H

#include "ch.h"

// Events sources
extern event_source_t deca_event; // sent on decawave IRQ rising edge
extern event_source_t imu_event; // sent on IMU IRQ rising edge

/* enable external interrupts */
void initExti(void);

#endif
