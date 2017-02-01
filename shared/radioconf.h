#ifndef RADIOCONF_H
#define RADIOCONF_H

/* Delay between ranging poll RX and response TX */
#define POLL_TO_RESP_DLY 800*UUS_TO_DWT_TIME

/* Delay between poll and RX activation */
#define POLL_TO_RESP_RX 600

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
#define MAX_CONNECTED_ROBOTS 15

// Radio message IDs
#define SOF_MSG_ID 0x23
#define BEACON_READ_MSG_ID 0x32
#define RANGING_MSG_ID 0x42
#define NEW_ROBOT_MSG_ID 0x2A

// flags bits :
#define RB_FLAGS_DEN 0x01   // dance enable
#define RB_FLAGS_WF 0x02    // write flash
#define RB_FLAGS_PTSTR 0x04 // store points
#define RB_FLAGS_CLSTR 0x08 // store colors
#define RB_FLAGS_CLR 0x10   // clear stored data

// status bits :
#define RB_STATUS_BATT 0x03 // battery state
#define RB_STATUS_WOK 0x04 // write OK

// battery state codes
#define BATTERY_VERYLOW 0
#define BATTERY_LOW 1
#define BATTERY_OK 2
#define BATTERY_HIGH 3

#endif
