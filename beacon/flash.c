/* The following code was greatly inspired and ported for STM32F302 from
* tegesoft's library (https://github.com/tegesoft/flash-stm32f407) */

#include "ch.h"
#include "flash.h"
#include <string.h>

typedef uint16_t flashdata_t;

// 2K pages
#define FLASH_PAGE_SIZE 2*1024
// returns the first address of a page
#define flashPageBegin(page) (FLASH_BASE + page*FLASH_PAGE_SIZE)
// Wait for the flash operation to finish
#define flashWaitWhileBusy() { while (FLASH->SR & FLASH_SR_BSY) {} }
// Lock the flash memory for write access.
#define flashLock() { FLASH->CR |= FLASH_CR_LOCK; }

// Unlock the flash memory for write access, return 0 if successful
static int flashUnlock(void) {
	/* Check if unlock is really needed */
	if(!(FLASH->CR & FLASH_CR_LOCK))
		return 0;

	/* Write magic unlock sequence */
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;

	/* Check if unlock was successful */
	if(FLASH->CR & FLASH_CR_LOCK)
		return 1;
	return 0;
}

int flashPageErase(uint8_t page) {
	// Unlock flash for write access
	if(flashUnlock() == 1)
		return FLASH_RETURN_NO_PERMISSION;

	// Wait for any busy flags
	flashWaitWhileBusy();

	// Start deletion of page
	FLASH->CR = FLASH_CR_PER; // page erase mode
	FLASH->AR  = flashPageBegin(page);
	FLASH->CR |= FLASH_CR_STRT;

	// make sure to wait at least 1 cycle before checking BSY flag
	asm volatile("nop");
	// Wait until it's finished
	flashWaitWhileBusy();

	// Page erase flag does not clear automatically
	FLASH->CR &= ~FLASH_CR_PER;

	// Lock flash again
	flashLock();

	// Check for errors
	if(FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR |= FLASH_SR_EOP; // clear EOP
		return FLASH_RETURN_SUCCESS;
	}

	return FLASH_RETURN_BAD_FLASH;  // Page is not empty despite the erase cycle!
}

static void flashWriteData(flashaddr_t address, const flashdata_t data) {
	// Enter flash programming mode
	FLASH->CR = FLASH_CR_PG;
	// Write the data
	*(flashdata_t*)address = data;
	// Wait for completion
	flashWaitWhileBusy();
	// Exit flash programming mode
	FLASH->CR &= ~FLASH_CR_PG;
	// clear EOP
	FLASH->SR |= FLASH_SR_EOP;
}

int flashWrite(flashaddr_t address, const char* buffer, size_t size) {
	// Unlock flash for write access
	if(flashUnlock() == 1)
		return FLASH_RETURN_NO_PERMISSION;

	// Wait for any busy flags
	flashWaitWhileBusy();

	/* Check if the flash address is correctly aligned */
	size_t alignOffset = address % sizeof(flashdata_t);
	if(alignOffset != 0) {
		/* Not aligned, thus we have to read the data in flash already present
		* and update them with buffer's data */

		// Align the flash address correctly
		flashaddr_t alignedFlashAddress = address - alignOffset;

		// Read already present data
		flashdata_t tmp = *(volatile flashdata_t*)alignedFlashAddress;

		// Compute how much bytes one must update in the data read
		size_t chunkSize = sizeof(flashdata_t) - alignOffset;
		if(chunkSize > size)
		chunkSize = size; // this happens when both address and address + size are not aligned

		// Update the read data with buffer's data
		memcpy((char*)&tmp + alignOffset, buffer, chunkSize);

		// Write the new data in flash
		flashWriteData(alignedFlashAddress, tmp);

		// Advance
		address += chunkSize;
		buffer += chunkSize;
		size -= chunkSize;
	}

	/* Now, address is correctly aligned. One can copy data directly from
	* buffer's data to flash memory until the size of the data remaining to be
	* copied requires special treatment. */
	while(size >= sizeof(flashdata_t)) {
		flashWriteData(address, *(const flashdata_t*)buffer);
		address += sizeof(flashdata_t);
		buffer += sizeof(flashdata_t);
		size -= sizeof(flashdata_t);
	}

	/* Now, address is correctly aligned, but the remaining data are too
	* small to fill a entier flashdata_t. Thus, one must read data already
	* in flash and update them with buffer's data before writing an entire
	* flashdata_t to flash memory. */
	if(size > 0) {
		flashdata_t tmp = *(volatile flashdata_t*)address;
		memcpy(&tmp, buffer, size);
		flashWriteData(address, tmp);
	}

	// Lock flash again
	flashLock();

	return FLASH_RETURN_SUCCESS;
}
