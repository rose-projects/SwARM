#ifndef RADIO_CONF_H
#define RADIO_CONF_H

/* Delay between ranging poll RX and response TX */
#define POLL_TO_RESP_DLY 600*UUS_TO_DWT_TIME

/* Delay between poll and RX activation */
#define POLL_TO_RESP_RX 400

/* RX timeout in us */
#define RX_TIMEOUT 1000

/* time waiting for start-of-frame in us */
#define SYNC_RX_TIMEOUT 50000

/* required time between RX enable and actual RX */
#define AHEAD_OF_TX_MARGIN 200*UUS_TO_DWT_TIME

/* total length of a frame in ms */
#define FRAME_LENGTH 100

/* time slot length in ms */
#define TIMESLOT_LENGTH 2

/* maximum supported number of connected robots */
#define MAX_CONNECTED_ROBOTS 48

// Radio message IDs
#define SOF_MSG_ID 0x23
#define BEACON_READ_MSG_ID 0x32
#define RANGING_MSG_ID 0x42
#define NEW_ROBOT_MSG_ID 0x2A

#endif
