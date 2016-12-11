#include "non-volatile.h"
#include "chprintf.h"

const int deviceUID;

/* set ID of the device (write it in flash).
 * USAGE: setid <NEW ID> */
void setDeviceUID(BaseSequentialStream *chp, int argc, char **argv) {
	(void) chp;
	(void) argc;
	(void) argv;
	//TODO
}

/* print ID of the device
 * USAGE: getid */
void getDeviceUID(BaseSequentialStream *chp, int argc, char **argv) {
	(void) argc;
	(void) argv;

	chprintf(chp, "Device ID = %d\n", deviceUID);
}
