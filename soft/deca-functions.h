#ifndef DECA_FUNTIONS_H
#define DECA_FUNTIONS_H

/* returns RX timestamp (40bit wide) */
#define getRXtimestamp() (((uint64_t) dwt_readrxtimestamplo32()) | (((uint64_t) dwt_readrxtimestamphi32()) << 8))

/* initialize the peripherals and the Decawave module */
int decaInit(void);

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
 *		noRXenable : 1 for no RX enable, useful when it will be enabled automatically after a TX
 * returns number of bytes read for successfully received message,
 *  	-1 for receive error, -2 for timeout, -3 for message too long
 */
int decaReceive(int maxSize, uint8_t *buffer, int noRXenable);

#endif
