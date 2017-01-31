#include <stdlib.h>
#include <string.h>
#include "ch.h"

#include "nonvolatile.h"
#include "../shared/flash.h"
#include "radiocomms.h"
#include "../shared/radioconf.h"
#include "usbconf.h"

#define MAX_CALIBRATION 50

/* ID of the beacon : 0 for mb, 253 for sb1, 254 for sb2 */
int deviceUID __attribute__((section(".flashdata")));
/* distance offsets in flash */
struct distOffset offsets[MAX_CALIBRATION] __attribute__((section(".flashdata")));

/* Flash data RAM instance to save data while erasing flash sector */
static int deviceUIDinRAM;
static struct distOffset offsetsInRAM[MAX_CALIBRATION];

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

struct distOffset* loadOffsets(uint32_t uid) {
	int i;
	struct distOffset offset = {
		uid, -200, -200, -200
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
