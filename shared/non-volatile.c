#include "ch.h"
#include "non-volatile.h"
#include "chprintf.h"
#include "flash.h"
#include "radio-conf.h"

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

const int deviceUID __attribute__((section(".flashdata")));

const int16_t mbOffsets[MAX_ROBOT_ID] __attribute__((section(".flashdata")));
const int16_t sb1Offsets[MAX_ROBOT_ID] __attribute__((section(".flashdata")));
const int16_t sb2Offsets[MAX_ROBOT_ID] __attribute__((section(".flashdata")));

void loadOffsets(void) {
	
}

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
