#include "ch.h"
#include "chprintf.h"
#include "chevents.h"

#include "usb-config.h"
#include "exti-config.h"
#include "deca-functions.h"
#include "decadriver/deca_device_api.h"
#include "decadriver/deca_regs.h"

// TX poll message
static uint8_t tx_poll[] = {0x41, 0x88, 0xDE, 0xCA};

// RX buffer
#define RX_BUF_LEN 12
static uint8_t rx_buffer[RX_BUF_LEN];

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 us and 1 us = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536
// Speed of light in air, in metres per second
#define SPEED_OF_LIGHT 299702547

// Delay between frames, in UWB microseconds
#define POLL_TO_RESP_DLY 800*UUS_TO_DWT_TIME
// abort receive if no message is received after RX_TIMEOUT
#define RX_TIMEOUT 5000
// Delay between poll and response
#define POLL_TO_RESP_RX 500

// returns the moving average of the 10 last mesured distances
static int mean(int newValue) {
	static int history[10];
	int i, result = 0;

	history[9] = newValue;
	for(i=0; i<10; i++) {
		if(i < 9)
			history[i] = history[i+1];
		result += history[i];
	}
	return result/10;
}

static void rangingTask(void) {
	int ret;

	while (1) {
		// Execute a delay between ranging exchanges
		chThdSleepMilliseconds(101);

		// make sure TX done bit is cleared
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

        // Send poll, and enable reception automatically to receive the answer
        decaSend(4, tx_poll, 1, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

		// receive response
        if((ret = decaReceive(RX_BUF_LEN, rx_buffer, 1)) < 0) {
			if(ret == -2)
				chprintf(USBserial, "timout\n");
			else
				chprintf(USBserial, "error\n");
			continue;
		}

        // check frame is actually our response
        if (rx_buffer[0] == 0x41 && rx_buffer[1] == 0x88) {
            int poll_tx_ts, resp_rx_ts, beacon_hold_time;
			int distance;

            // Retrieve poll transmission and response reception timestamps
            poll_tx_ts = dwt_readtxtimestamplo32();
            resp_rx_ts = dwt_readrxtimestamplo32();

            // compute precise time between beacon poll RX and response TX
            beacon_hold_time = POLL_TO_RESP_DLY + TX_ANT_DLY - ((int) rx_buffer[2]) - ((int) (rx_buffer[3]&0x01) << 8);

			// compute distance
            distance = (resp_rx_ts - poll_tx_ts - beacon_hold_time)*1000/4264;

        	chprintf(USBserial, "DIST: %d cm\r", mean(distance));
        }
    }
}

static void beaconTask(void) {
	uint64_t resp_msg, rx_ts;

	while (1) {
		if(decaReceive(RX_BUFFER_LEN, rx_buffer, 0) < 0) {
			//chprintf(USBserial, "RX error\n");
		} else if(rx_buffer[0] == 0x41 && rx_buffer[1] == 0x88) { // Check that the frame is a poll
			// Retrieve poll reception timestamp
			rx_ts = getRXtimestamp();

			// set response message transmission time
			dwt_setdelayedtrxtime((rx_ts + POLL_TO_RESP_DLY) >> 8);

			// Write timestamp in the final message
			resp_msg = (rx_ts << 16) + (0x8841);

			// send the response message
			if(decaSend(7, (uint8_t*) &resp_msg, 1, DWT_START_TX_DELAYED) < 0) {
				//chprintf(USBserial, "TX error\n");
			}
		}

		chThdSleepMilliseconds(100);
	}
}

int main(void) {
	int i;
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
	// Set expected response's delay and timeout
	dwt_setrxaftertxdelay(POLL_TO_RESP_RX);
	dwt_setrxtimeout(RX_TIMEOUT);

	chEvtRegisterMask(&deca_event, &evt_listener, EVENT_MASK(0));

	// scan for a beacon, if found one become a ranging master, otherwise become a beacon
	for(i=0; i<100; i++) {
		decaSend(4, tx_poll, 1, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

		// receive response
        if(decaReceive(RX_BUF_LEN, rx_buffer, 1) > 0) {
			rangingTask();
		}

		chThdSleepMilliseconds(1);
	}

    beaconTask();
	return 0;
}
