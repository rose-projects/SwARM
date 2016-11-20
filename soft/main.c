#include "ch.h"
#include "chprintf.h"
#include "chevents.h"

#include "usb-config.h"
#include "exti-config.h"
#include "deca-functions.h"
#include "decadriver/deca_device_api.h"
#include "decadriver/deca_regs.h"

/* RX buffer */
#define RX_BUF_LEN 12
static uint8 rx_buffer[RX_BUF_LEN];

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 us and 1 us = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

/* Delay between frames, in UWB microseconds.*/
#define POLL_TO_RESP_DLY 1000*UUS_TO_DWT_TIME // TODO : understand why this doesn't seem to be microseconds

int main(void) {
	uint64_t rx_ts;
	event_listener_t evt_listener;

	// initialize ChibiOS
    halInit();
    chSysInit();
	// initialize interrupt on decawave IRQ
	initExti();
    // initialize serial over USB
    initUSB();
	// initialize decawave module
	decaInit();

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

    while (1) {
        if(decaReceive(RX_BUFFER_LEN, rx_buffer) < 0) {
			chprintf(USBserial, "RX error\n");
			continue;
		}

        // Check that the frame is a poll
        if(rx_buffer[0] == 0x41 && rx_buffer[1] == 0x88) {
            // Retrieve poll reception timestamp
            rx_ts = getRXtimestamp();

            // set response message transmission time
            dwt_setdelayedtrxtime((rx_ts + POLL_TO_RESP_DLY) >> 8);

            // Write timestamp in the final message
            rx_ts = (rx_ts << 16) + (0x8841);

            // send the response message
            if(decaSend(9, (uint8_t*) &rx_ts, 1, 1) < 0)
				chprintf(USBserial, "TX error\n");
        }

		chThdSleepMilliseconds(97);
    }
}
