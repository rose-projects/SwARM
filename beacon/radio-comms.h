#ifndef RADIO_COMMS_H
#define RADIO_COMMS_H

#include "ch.h"

/* event triggered when new robot locations are available */
extern event_source_t radioEvent;

/* initialize decawave module and start radio communication thread */
void startRadio(void);

/* shell callback, USAGE : list */
void dumpConnectedDevices(BaseSequentialStream *chp, int argc, char **argv);

/* reset master beacon radio communication 
 * (useful after a flash write) */
void restartRadio(void);

#endif
