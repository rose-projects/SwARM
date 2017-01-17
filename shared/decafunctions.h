#ifndef DECAFUNTIONS_H
#define DECAFUNTIONS_H

/* Default antenna delay values for 64 MHz PRF */
#define TX_ANT_DLY 16393
#define RX_ANT_DLY 16393

/* Speed of light in air, in metres per second */
#define SPEED_OF_LIGHT 299702547

/* millisecond to decawave time unit conversion factor */
#define MS_TO_DWT ((int64_t) 1000*UUS_TO_DWT_TIME)

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 us and 1 us = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

/* returns RX timestamp (40bit wide) */
#define getRXtimestamp() (((uint64_t) dwt_readrxtimestamplo32()) | (((uint64_t) dwt_readrxtimestamphi32()) << 8))

/* returns RX timestamp (40bit wide) */
#define getTXtimestamp() (((uint64_t) dwt_readtxtimestamplo32()) | (((uint64_t) dwt_readtxtimestamphi32()) << 8))

/* initialize the peripherals and the Decawave module */
int decaInit(void);

/* switch SPI speed to 10.4MHz*/
void useFastSPI(void);

/* change radio channel used for transmissions */
#define MB_CHANNEL 2
#define SB1_CHANNEL 3
#define SB2_CHANNEL 4
void switchToChannel(int channel);

/* send a message :
 * 		size : the size of the message in bytes
 *		buffer : pointer to the message data
 *		ranging : 1 if message is used for ranging (i.e. timestamps will be used), 0 otherwise
 *		flags : tx flags (see decadriver's dwt_starttx)
 * returns 0 for success, -1 for failure (see decadriver's dwt_starttx)
 */
int decaSend(int size, uint8_t *buffer, int ranging, int flags);

/* activate receiver and wait for a message
 * 		maxSize : the maximum length of the message to receive
 * 		buffer : buffer to write the message in
 *		flag : DWT_START_RX_IMMEDIATE, DWT_START_RX_DELAYED or NO_RX_ENABLE
 * returns number of bytes read for successfully received message,
 *  	-1 for receive error, -2 for timeout, -3 for message too long
 */
#define NO_RX_ENABLE -1
int decaReceive(int maxSize, uint8_t *buffer, int flag);

/* wait until period (in UWB ms) has elapsed after previous (in systick) */
void sleepUntil(systime_t previous, int period);

#endif
