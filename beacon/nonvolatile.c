#include <stdlib.h>
#include <string.h>
#include "ch.h"

#include "nonvolatile.h"
#include "../shared/flash.h"
#include "radiocomms.h"
#include "../shared/radioconf.h"
#include "usbconf.h"

#define MAX_CALIBRATION 20

// ID of the beacon : 0 for mb, 253 for sb1, 254 for sb2
int deviceUID __attribute__((section(".flashdata")));
// decawave modules calibration data
struct distOffset offsets[MAX_CALIBRATION] __attribute__((section(".flashdata")));

// RAM buffers to save data while erasing flash page
static int deviceUIDinRAM;
static struct distOffset offsetsInRAM[MAX_CALIBRATION];

// copy data from flash to buffers in RAM and erase flash page
static int saveAndErase(void) {
	int ret;

	// save data from flash to RAM
	deviceUIDinRAM = deviceUID;
	memcpy(offsetsInRAM, offsets, sizeof(offsets));

	// erase page
	chSysLock();
	ret = flashPageErase(FLASHDATA_PAGE);
	chSysUnlock();

	return ret;
}

// write data in flash from buffers in RAM
static int writeFlash(void) {
	int ret;

	// write deviceUID in flash from deviceUIDinRAM
	chSysLock();
	ret = flashWrite((flashaddr_t) &deviceUID, (char*) &deviceUIDinRAM, sizeof(deviceUID));
	chSysUnlock();
	if(ret)
		return ret;

	// write offsets in flash from offsetsInRAM
	chSysLock();
	ret = flashWrite((flashaddr_t) offsets, (char*) offsetsInRAM, sizeof(offsets));
	chSysUnlock();
	return ret;
}

// load calibration data stored in flash (and create an entry if necessary)
struct distOffset* loadOffsets(uint32_t uid) {
	int i;
	struct distOffset offset = {
		uid, -200, -200, -200, 1000, 1000, 1000
	};

	for(i = 0; i<MAX_CALIBRATION; i++)
		if(offsets[i].uid == uid)
			return &offsets[i];

	// no matching calibration data found : create an entry
	i = writeOffset(&offset);
	if(i >= 0)
		return &offsets[i];
	else
		return &offsets[0];
}

// write calibration data
int writeOffset(struct distOffset *offset) {
	int i = 0;

	// search for an old calibration matching the UID or an empty slot
	while(i < MAX_CALIBRATION && offsets[i].uid != offset->uid && offsets[i].uid != 0xFFFFFFFF)
		i++;

	if(i == MAX_CALIBRATION) {
		printf("calibration buffer is full\n");
		return -1;
	}

	if(saveAndErase()) {
		printf("Couln't erase flash.\n");
		return -1;
	}
	offsetsInRAM[i].uid = offset->uid;
	offsetsInRAM[i].mb = offset->mb;
	offsetsInRAM[i].sb1 = offset->sb1;
	offsetsInRAM[i].sb2 = offset->sb2;
	offsetsInRAM[i].mbCoeff = offset->mbCoeff;
	offsetsInRAM[i].sb1Coeff = offset->sb1Coeff;
	offsetsInRAM[i].sb2Coeff = offset->sb2Coeff;
	if(writeFlash()) {
		printf("Couln't write flash.\n");
		return -1;
	}

	// restart radio as it can't recover from such a long blocking
	restartRadio();

	return i;
}

/* set ID of the device (write it in flash).
* USAGE: setid <NEW ID> */
void setDeviceUID(BaseSequentialStream *chp, int argc, char **argv) {
	(void) chp;
	(void) argc;
	(void) argv;

	if(argc != 1) {
		chprintf(chp, "USAGE: setid <NEW ID>\n");
		return;
	}

	if(saveAndErase()) {
		chprintf(chp, "Couln't erase flash.\n");
		return;
	}
	deviceUIDinRAM = atoi(argv[0]);
	if(writeFlash()) {
		chprintf(chp, "Couln't write flash.\n");
		return;
	}

	// restart radio as it can't recover from such a long blocking
	restartRadio();

	chprintf(chp, "OK\n");
}

/* print ID of the device
* USAGE: getid */
void getDeviceUID(BaseSequentialStream *chp, int argc, char **argv) {
	(void) argc;
	(void) argv;

	chprintf(chp, "Device ID = %d\n", deviceUID);
}
