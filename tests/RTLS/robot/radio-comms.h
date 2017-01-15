#ifndef RADIO_COMMS_H
#define RADIO_COMMS_H

/* container for data received/sent to master beacon */
struct robotData {
	// sent to the robot
	uint8_t H;
	uint8_t S;
	uint8_t V;
	uint16_t x;
	uint16_t y;
	uint16_t goalX;
	uint16_t goalY;
	uint8_t goalSpeed;
	uint8_t flags;
	// sent to master beacon
	uint8_t status;
};

/* last data received/to send */
extern struct robotData radioData;

/* event triggered when new robot locations are available */
extern event_source_t radioEvent;

/* initialize decawave module and start radio communication thread */
void startRadio(void);

#endif
