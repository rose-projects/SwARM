#ifndef RADIOCOMMS_H
#define RADIOCOMMS_H

#include "ch.h"
#include "chvt.h"
#include "chevents.h"

/* container for data received/sent to master beacon */
struct robotData {
	// sent to the robot
	int16_t x;
	int16_t y;
	uint8_t flags;
	// sent to master beacon
	uint8_t status;
};

/* last data received/to send */
extern struct robotData radioData;

/* event triggered when new robot data has been received */
extern event_source_t radioEvent;

/* initialize decawave module and start radio communication thread */
void startRadio(void);

/* get date (for dance sync) in 0.1s (returns 0 if the radio in not synchronized) */
float getDate(void);

#endif // RODIOCOMMS_H
