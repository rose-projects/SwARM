#ifndef RADIO_COMMS_H
#define RADIO_COMMS_H

#include "ch.h"

/* event triggered when new robot locations are available */
extern event_source_t radio_event;

/* initialize decawave module and start radio communication thread */
void startRadio(void);

#endif
