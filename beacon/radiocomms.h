#ifndef RADIOCOMMS_H
#define RADIOCOMMS_H

#include "ch.h"

/* event triggered when new robot locations are available */
extern event_source_t radioEvent;
/* event triggered when payload has been sent and robot's answer has been received */
extern event_source_t payloadEvent;

/* initialize decawave module and start radio communication thread */
void startRadio(void);

/* shell callback, USAGE : list */
void dumpConnectedDevices(BaseSequentialStream *chp, int argc, char **argv);

/* reset master beacon radio communication (useful after a flash write) */
void restartRadio(void);

/* reset date counter (for dance timing sync) */
void resetDate(void);

#endif
