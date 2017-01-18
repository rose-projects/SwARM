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

/** @brief Address in the flash memory */
typedef uintptr_t flashaddr_t;

/**
 * @brief Erase the flash @p page.
 * @details The page is checked for errors after erase.
 * @note The page is deleted regardless of its current state.
 *
 * @param page Page which is going to be erased.
 * @return FLASH_RETURN_SUCCESS         No error erasing the page.
 * @return FLASH_RETURN_BAD_FLASH       Flash cell error.
 * @return FLASH_RETURN_NO_PERMISSION   Access denied.
 */
int flashPageErase(uint8_t page);

/**
 * @brief Copy data from a @p buffer to the flash memory.
 * @warning The flash memory area receiving the data must be erased.
 * @warning The @p buffer must be at least @p size bytes long.
 * @param address First address in the flash memory where to copy the data to.
 * @param buffer Buffer containing the data to copy.
 * @param size Size of the data to be copied in bytes.
 * @return FLASH_RETURN_SUCCESS         No error.
 * @return FLASH_RETURN_NO_PERMISSION   Access denied.
 */
int flashWrite(flashaddr_t address, const char* buffer, size_t size);

#endif /* FLASH_H */
