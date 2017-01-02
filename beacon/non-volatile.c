#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "chprintf.h"

#include "non-volatile.h"
#include "flash.h"
#include "radio-comms.h"
#include "../shared/radio-conf.h"
#include "../shared/usb-config.h"

#define MAX_CALIBRATION 50

// The flash_sector_def struct describes a flash sector
struct flash_sector_def {
   flashsector_t index;      // Index of the sector in the flash memory
   size_t size;              // Size of the sector in bytes
   flashaddr_t beginAddress; // First address of the sector in memory (inclusive)
   flashaddr_t endAddress;   // Last address of the sector in memory (exclusive)
};

// Definition of the stm32f407 sectors.
const struct flash_sector_def f407_flash[FLASH_SECTOR_COUNT] = {
    {  0,  16 * 1024, 0x08000000, 0x08004000},
    {  1,  16 * 1024, 0x08004000, 0x08008000},
    {  2,  16 * 1024, 0x08008000, 0x0800C000},
    {  3,  16 * 1024, 0x0800C000, 0x08010000},
    {  4,  64 * 1024, 0x08010000, 0x08020000},
    {  5, 128 * 1024, 0x08020000, 0x08040000},
    {  6, 128 * 1024, 0x08040000, 0x08060000},
    {  7, 128 * 1024, 0x08060000, 0x08080000},
    {  8, 128 * 1024, 0x08080000, 0x080A0000},
    {  9, 128 * 1024, 0x080A0000, 0x080C0000},
    { 10, 128 * 1024, 0x080C0000, 0x080E0000},
    { 11, 128 * 1024, 0x080E0000, 0x08100000}
};

/* ID of the beacon : 0 for mb, 253 for sb1, 254 for sb2 */
int deviceUID __attribute__((section(".flashdata")));

/* distance offsets in flash */
static struct distOffset offsets[MAX_CALIBRATION] __attribute__((section(".flashdata")));

/* Flash data RAM instance to save data while erasing flash sector */
static int deviceUIDinRAM;
static struct distOffset offsetsInRAM[MAX_CALIBRATION];

static int saveAndErase(void) {
	int ret;

	// save data from flash to RAM
	deviceUIDinRAM = deviceUID;
	memcpy(offsetsInRAM, offsets, sizeof(offsets));

	// erase sector
	chSysLock();
	ret = flashSectorErase(f407_flash[11].index);
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
	while(i < MAX_CALIBRATION && offsets[i].uid != offset->uid && offsets[i].uid != 0)
		i++;

	if(i == MAX_CALIBRATION) {
		chprintf(USBserial, "calibration buffer is full\n");
		return -1;
	}

	if(saveAndErase()) {
		chprintf(USBserial, "Couln't erase flash.\n");
		return -1;
	}
	offsetsInRAM[i].uid = offset->uid;
	offsetsInRAM[i].mb = offset->mb;
	offsetsInRAM[i].sb1 = offset->sb1;
	offsetsInRAM[i].sb2 = offset->sb2;
	if(writeFlash()) {
		chprintf(USBserial, "Couln't write flash.\n");
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
