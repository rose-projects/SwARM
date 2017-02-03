#include <string.h>
#include "ch.h"
#include "hal.h"
#include "chevents.h"

#include "../shared/flash.h"
#include "../shared/radioconf.h"
#include "RTT/SEGGER_RTT.h"
#include "radiocomms.h"
#include "dance.h"
#include "coordination.h"

#define MAX_MOVE_POINTS 64
#define MAX_COLOR_POINTS 110

const int ADVANCE_TIME = 2; // in 0.1s

// RAM buffers storing data to be written in flash
struct move movesBuffer[MAX_MOVE_POINTS];
struct color colorsBuffer[MAX_COLOR_POINTS];

int storedMoves = 0;
int storedColors = 0;

// dance data in flash
struct move danceMoves[MAX_MOVE_POINTS] __attribute__((section(".flashdata")));
struct color danceColors[MAX_COLOR_POINTS] __attribute__((section(".flashdata")));

int danceMovesCnt __attribute__((section(".flashdata")));
int danceColorsCnt __attribute__((section(".flashdata")));

// pointers to the current steps in the dance
struct move *currentMove;
struct color *currentColor;

void clearStoredData(void) {
	storedMoves = 0;
	storedColors = 0;
	radioData.status &= ~RB_STATUS_WOK;
}

void writeStoredData(void) {
	int ret;
	// erase flash
	chSysLock();
	ret = flashPageErase(FLASHDATA_PAGE);

	// write moves in flash
	ret += flashWrite((flashaddr_t) danceMoves, (char*) movesBuffer, sizeof(struct move)*storedMoves);
	ret += flashWrite((flashaddr_t) &danceMovesCnt, (char*) &storedMoves, sizeof(storedMoves));

	// write colors in flash
	ret += flashWrite((flashaddr_t) danceColors, (char*) colorsBuffer, sizeof(struct color)*storedColors);
	ret += flashWrite((flashaddr_t) &danceColorsCnt, (char*) &storedColors, sizeof(storedColors));
	chSysUnlock();

	storedMoves = 0;
	storedColors = 0;

	if(ret != 0)
		printf("Something went wrong while flashing dance\n");
}

void storeMoves(uint8_t* buffer, int pointCnt) {
	int i;

	// check there's enough room
	if(storedMoves+pointCnt > MAX_MOVE_POINTS) {
		printf("too many points !\n");
		return;
	}

	// copy data
	for(i=0; i < pointCnt; i++) {
		movesBuffer[storedMoves + i].date =  buffer[i*11] + (buffer[i*11 + 1] << 8);
		movesBuffer[storedMoves + i].x =  buffer[i*11 + 2] + (buffer[i*11 + 3] << 8);
		movesBuffer[storedMoves + i].y =  buffer[i*11 + 4] + (buffer[i*11 + 5] << 8);
		movesBuffer[storedMoves + i].angle =  buffer[i*11 + 6];
		movesBuffer[storedMoves + i].startRadius =  buffer[i*11 + 7] + (buffer[i*11 + 8] << 8);
		movesBuffer[storedMoves + i].endRadius =  buffer[i*11 + 9] + (buffer[i*11 + 10] << 8);
	}
	storedMoves += pointCnt;
}
void storeColors(uint8_t* buffer, int pointCnt) {
	int i;

	// check there's enough room
	if(storedColors+pointCnt > MAX_COLOR_POINTS) {
		printf("too many colors points !\n");
		return;
	}

	// copy data
	for(i=0; i < pointCnt; i++) {
		colorsBuffer[storedColors + i].date =  buffer[i*6] + (buffer[i*6 + 1] << 8);
		colorsBuffer[storedColors + i].h =  buffer[i*6 + 2];
		colorsBuffer[storedColors + i].s =  buffer[i*6 + 3];
		colorsBuffer[storedColors + i].v =  buffer[i*6 + 4];
		colorsBuffer[storedColors + i].fadeTime =  buffer[i*6 + 5];
	}
	storedColors += pointCnt;
}

void saveDance(void) {
	storedMoves = danceMovesCnt > MAX_MOVE_POINTS ? 0 : danceMovesCnt;
	storedColors = danceColorsCnt > MAX_COLOR_POINTS ? 0 : danceColorsCnt;

	memcpy(movesBuffer, danceMoves, sizeof(struct move)*danceMovesCnt);
	memcpy(colorsBuffer, danceColors, sizeof(struct color)*danceColorsCnt);
}

static THD_WORKING_AREA(waSequencer, 256);
static THD_FUNCTION(sequencerThread, th_data) {
	event_listener_t evt_listener;
	int i, date;

	(void) th_data;
	chRegSetThreadName("Sequencer");

	while(1) {
		// if dance is enabled
		if(radioData.flags & RB_FLAGS_DEN) {
			date = getDate();

			// find the next point to execute
			i = 0;
			// + ADVANCE_TIME to load the next point in advance
			while(i < danceMovesCnt &&
			      danceMoves[i].date + ADVANCE_TIME < date)
			{
				i++;
			}
			// if found, set as the current goal
			if(i < danceMovesCnt && &danceMoves[i] != currentMove) {
				currentMove = &danceMoves[i];
				update_main_coordinates();
			}

			// find the next color to display
			i = 0;
			while(i < danceColorsCnt && danceColors[i].date < date)
				i++;
			// if found and we have to start fading, set as the current goal
			if(i < danceColorsCnt &&
			   (danceColors[i].date - danceColors[i].fadeTime) <= date)
			{
				currentColor = &danceColors[i];
			}
		} else {
			currentColor = &danceColors[0];
			currentMove = &danceMoves[0];
		}
		chThdSleepMilliseconds(50);
	}
}

void initSequencer(void) {
	// setup pointers
	currentColor = &danceColors[0];
	currentMove = &danceMoves[0];

	// start sequencer
	chThdCreateStatic(waSequencer, sizeof(waSequencer), NORMALPRIO-2, sequencerThread, NULL);
}
