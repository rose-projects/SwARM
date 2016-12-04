#include "ch.h"
#include "chvt.h"
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
#define POLL_TO_RESP_DLY 600*UUS_TO_DWT_TIME
// abort receive if no message is received after RX_TIMEOUT
#define RX_TIMEOUT 5000
// Delay between poll and response
#define POLL_TO_RESP_RX 400

// returns the moving average of the MEAN_LEN last mesured distances
#define MEAN_LEN 20
static int mean(int newValue) {
	static int history[MEAN_LEN];
	int i, result = 0, min = 100000, max = 0;

	history[MEAN_LEN-1] = newValue;
	for(i=0; i<MEAN_LEN; i++) {
		if(history[i] < min)
			min = history[i];
		if(history[i] > max)
			max = history[i];
		if(i < MEAN_LEN - 1)
			history[i] = history[i+1];
		result += history[i];
	}

	// show min and max of the history to estimate noise
	chprintf(USBserial, "max = %d, min =  %d, noise = %d\n", max, min, max-min);
	return result/MEAN_LEN;
}

static void rangingTask(void) {
	int ret;
	systime_t start;

	while (1) {
		// Execute a delay between ranging exchanges
		chThdSleepMilliseconds(101);

		start = chVTGetSystemTime();
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
			double distance;
			int32_t tx_ts, rx_ts, beacon_rx_ts, beacon_hold_time;
			int a;

            // Retrieve poll transmission and response reception timestamps
            tx_ts = dwt_readtxtimestamplo32();
            rx_ts = dwt_readrxtimestamplo32();

			// retrieve beacon RX timestamp
			beacon_rx_ts = (int) rx_buffer[2] + ((int) rx_buffer[3] << 8) +
				((int) rx_buffer[4] << 16) + ((int) rx_buffer[5] << 24);
            // compute precise time between beacon poll RX and response TX
            beacon_hold_time = POLL_TO_RESP_DLY + TX_ANT_DLY - (beacon_rx_ts & 0x1FF);

			// compute distance
            distance = (rx_ts - tx_ts - beacon_hold_time) * 100 / 2.0 * DWT_TIME_UNITS * SPEED_OF_LIGHT;
			a = distance;
        	chprintf(USBserial, "DIST: %d cm\r", mean(a));
			chprintf(USBserial, "duration: %d us\r", ST2US(chVTGetSystemTime()-start));
        }
    }
}

static void beaconTask(void) {
	uint64_t rx_ts, resp_msg;

	while (1) {
		chThdSleepMilliseconds(100);

		if(decaReceive(RX_BUFFER_LEN, rx_buffer, 0) < 0)
			continue;

		// Check that the frame is a poll
		if(rx_buffer[0] != 0x41 || rx_buffer[1] != 0x88)
			continue;

		// Retrieve poll reception timestamp
		rx_ts = getRXtimestamp();

		// set response message transmission time
		dwt_setdelayedtrxtime((rx_ts + POLL_TO_RESP_DLY) >> 8);

		// Write timestamp in the final message
		resp_msg = (rx_ts << 16) + (0x8841);

		// send the response message
		decaSend(7, (uint8_t*) &resp_msg, 1, DWT_START_TX_DELAYED);
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
