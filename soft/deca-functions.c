#include "ch.h"
#include "chvt.h"
#include "chprintf.h"
#include "chevents.h"

#include "usb-config.h"
#include "exti-config.h"
#include "deca-platform.h"
#include "decadriver/deca_device_api.h"
#include "decadriver/deca_regs.h"

// Default communication configuration
static dwt_config_t config = {
    2,               // Channel number
    DWT_PRF_64M,     // Pulse repetition frequency
    DWT_PLEN_128,    // Preamble length. Used in TX only
    DWT_PAC8,        // Preamble acquisition chunk size. Used in RX only
    9,               // TX preamble code. Used in TX only
    9,               // RX preamble code. Used in RX only
    0,               // 0 to use standard SFD, 1 to use non-standard SFD
    DWT_BR_6M8,      // Data rate
    DWT_PHRMODE_EXT, // PHY header mode
    (129 + 8 - 8)    // SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only
};

// Default antenna delay values for 64 MHz PRF
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436


int decaInit(void) {
	// initialize peripherals used by decawave module
    initDecaPlatform();
	// reset module
	dwt_softreset();
	chThdSleepMilliseconds(3);
	// initialize for ranging
	if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        return -1;
    }
    dwt_configure(&config);
    // Apply default antenna delay value.
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
	// activate interrupts for tx done, rx done, rx errors
	dwt_setinterrupt(SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_TXFRS, 1);

	return 0;
}

int decaSend(int size, uint8_t *buffer, int ranging, int delayed) {
	int ret;
	dwt_writetxdata(size + 2, buffer, 0);
	dwt_writetxfctrl(size + 2, 0, ranging);
	ret = dwt_starttx(delayed ? DWT_START_TX_DELAYED : DWT_START_TX_IMMEDIATE);

	if(ret == DWT_SUCCESS) {
		// wait until message is sent
		chEvtWaitAny(ALL_EVENTS);
		// Clear TXFRS event
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
	}
	return ret;
}

int decaReceive(int maxSize, uint8_t *buffer) {
	int messageLength;
	uint32_t status_reg;

	// activate reception
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	chEvtWaitAny(ALL_EVENTS);
	status_reg = dwt_read32bitreg(SYS_STATUS_ID);

	if (status_reg & SYS_STATUS_RXFCG) {
		// Clear good RX frame event in the DW1000 status register.
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

		// read the message
		messageLength = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		if (messageLength <= maxSize) {
			dwt_readrxdata(buffer, messageLength, 0);
			return messageLength;
		} else {
			return -2; // error : buffer is too small
		}
	} else {
		// Clear RX error events in the DW1000 status register.
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
		// Reset RX to properly reinitialise LDE operation. (User Manual p.34)
		dwt_rxreset();
		return -1;
	}
}
