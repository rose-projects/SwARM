/* The following code was greatly inspired and ported for STM32F302 from
 * tegesoft's library (https://github.com/tegesoft/flash-stm32f407) */

#ifndef FLASH_H
#define FLASH_H

#include "ch.h"
#include "hal.h"
#include <stdint.h>

/* ######### Error codes ######### */
/* Flash operation successful */
#define FLASH_RETURN_SUCCESS 0
/* Flash operation error because of denied access, corrupted memory.*/
#define FLASH_RETURN_NO_PERMISSION -1
/* Flash operation error because of bad flash, corrupted memory */
#define FLASH_RETURN_BAD_FLASH -11

#define FLASHDATA_PAGE 15

/* Address in the flash memory */
typedef uintptr_t flashaddr_t;

/* Erase the flash page.
 * The page is checked for errors after erase.
 * Note:  The page is deleted regardless of its current state.
 *
 * argument : page Page which is going to be erased.
 * returns : FLASH_RETURN_SUCCESS         No error erasing the page.
 *           FLASH_RETURN_BAD_FLASH       Flash cell error.
 *           FLASH_RETURN_NO_PERMISSION   Access denied.
 */
int flashPageErase(uint8_t page);

/* Copy data from a @p buffer to the flash memory.
 * Warning : * The flash memory area receiving the data must be erased.
 *           * buffer must be at least size bytes long.
 * arguments : address First address in the flash memory where to copy the data to.
 *             buffer Buffer containing the data to copy.
 *             size Size of the data to be copied in bytes.
 * returns : FLASH_RETURN_SUCCESS         No error.
 *           FLASH_RETURN_NO_PERMISSION   Access denied.
 */
int flashWrite(flashaddr_t address, const char* buffer, size_t size);

#endif /* FLASH_H */
