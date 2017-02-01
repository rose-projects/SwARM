#ifndef NONVOLATILE_H
#define NONVOLATILE_H

#include "ch.h"

/* container for distance calibration data (for a robot and with each beacon) */
struct distOffset {
	uint32_t uid; // UID of the robot
	int16_t mb;   // offset from the master beacon in cm
	int16_t sb1;  // offset from the slave beacon 1 in cm
	int16_t sb2;  // offset from the slave beacon 2 in cm
	int16_t mbCoeff;   // linear coefficient from the master beacon (in 1/1000)
	int16_t sb1Coeff;  // linear coefficient from the slave beacon 1 (in 1/1000)
	int16_t sb2Coeff;  // linear coefficient from the slave beacon 2 (in 1/1000)
};

/* ID of the beacon : 0 for mb, 253 for sb1, 254 for sb2 */
extern int deviceUID;

/* retrieve the distance offsets of a robot */
struct distOffset* loadOffsets(uint32_t uid);

/* write an offset to the flash */
int writeOffset(struct distOffset *offset);

/* ############## Shell command callbacks ############## */

/* set ID of the device (write it in flash).
 * USAGE: setid <NEW ID> */
void setDeviceUID(BaseSequentialStream *chp, int argc, char **argv);
/* print ID of the device
 * USAGE: getid */
void getDeviceUID(BaseSequentialStream *chp, int argc, char **argv);

#endif
