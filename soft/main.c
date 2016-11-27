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
#define MEAN_LEN 3
static int mean(int newValue) {
	static int history[MEAN_LEN];
	int i, result = 0;

	history[MEAN_LEN-1] = newValue;
	for(i=0; i<MEAN_LEN; i++) {
		if(i < MEAN_LEN - 1)
			history[i] = history[i+1];
		result += history[i];
	}
	return result/MEAN_LEN;
}

static int rangingResponse(uint32_t *poll_ts, int flags, int noRXenable) {
	uint64_t resp_msg, rx_ts;

	if(decaReceive(RX_BUFFER_LEN, rx_buffer, noRXenable) < 0) {
		//chprintf(USBserial, "RX error\n");
		return -2;
	}
	// Check that the frame is a poll
	if(rx_buffer[0] != 0x41 || rx_buffer[1] != 0x88)
		return -3;

	// Retrieve poll reception timestamp
	rx_ts = getRXtimestamp();

	// set response message transmission time
	dwt_setdelayedtrxtime((rx_ts + POLL_TO_RESP_DLY) >> 8);

	// Write timestamp in the final message
	resp_msg = (rx_ts << 16) + (0x8841);

	*poll_ts = rx_ts;
	// send the response message
	return decaSend(7, (uint8_t*) &resp_msg, 1, DWT_START_TX_DELAYED | flags);
}

static void rangingTask(void) {
	int ret;
	uint32_t tx_ts1, rx_ts1, tx_ts2, rx_ts2, beacon_hold_time1, beacon_hold_time2;

	while (1) {
		// Execute a delay between ranging exchanges
		chThdSleepMilliseconds(101);

		// make sure TX done bit is cleared
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

        // Send poll, and enable reception automatically to receive the answer
        decaSend(4, tx_poll, 1, DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
		tx_ts1 = dwt_readtxtimestamplo32(); // get TX timestamp

		// wait for answer and reply
		if((ret = rangingResponse(&rx_ts1, DWT_RESPONSE_EXPECTED, 1)) < 0) {
			if(ret == -1)
				chprintf(USBserial, "TX too late\n");
			else
				chprintf(USBserial, "RX error\n");
			continue;
		}

		// compute precise time between beacon poll RX and response TX
		beacon_hold_time1 = POLL_TO_RESP_DLY + TX_ANT_DLY - ((int) rx_buffer[2]) - ((int) (rx_buffer[3]&0x01) << 8);

		// receive 2nd response
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
			int a;
            // Retrieve poll transmission and response reception timestamps
            tx_ts2 = dwt_readtxtimestamplo32();
            rx_ts2 = dwt_readrxtimestamplo32();

            // compute precise time between beacon poll RX and response TX
            beacon_hold_time2 = POLL_TO_RESP_DLY + TX_ANT_DLY - ((int) rx_buffer[2]) - ((int) (rx_buffer[3]&0x01) << 8);
			
			// compute distance
            distance = (rx_ts1 - tx_ts1 - beacon_hold_time1 + rx_ts2 - tx_ts2 - beacon_hold_time2) * 25.0 * DWT_TIME_UNITS * SPEED_OF_LIGHT;
			a = distance;
        	chprintf(USBserial, "DIST: %d cm\r", mean(a));
        }
    }
}

static void beaconTask(void) {
	uint32_t rx_ts;

	while (1) {
		chThdSleepMilliseconds(100);
		if(rangingResponse(&rx_ts, DWT_RESPONSE_EXPECTED, 0) < 0)
			continue;
		rangingResponse(&rx_ts, 0, 1);
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
