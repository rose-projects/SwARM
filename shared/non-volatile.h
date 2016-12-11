#ifndef NON_VOLATILE_H
#define NON_VOLATILE_H

#include "ch.h"

extern const int deviceUID;

/* ############## Shell command callbacks ############## */

/* set ID of the device (write it in flash).
 * USAGE: setid <NEW ID> */
void setDeviceUID(BaseSequentialStream *chp, int argc, char **argv);
/* print ID of the device
 * USAGE: getid */
void getDeviceUID(BaseSequentialStream *chp, int argc, char **argv);

#endif
