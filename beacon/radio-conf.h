#ifndef RADIO_CONF_H
#define RADIO_CONF_H

/* device UID */
//#define DEVICE_UID 0 // master beacon
#define DEVICE_UID 253 // slave beacon 1
//#define DEVICE_UID 254 // slave beacon 2

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

/* maximum supported robot ID */
#define MAX_ROBOT_ID 50

#endif
