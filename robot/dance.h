#ifndef DANCE_H
#define DANCE_H

/* empty dance point storages in RAM */
void clearStoredData(void);
/* write dance downloaded in RAM to the flash */
void writeStoredData(void);
/* store dance points before writing them */
void storeMoves(uint8_t* buffer, int pointCnt);
/* store dance color points before writing them */
void storeColors(uint8_t* buffer, int pointCnt);
/* copy dance from flash to RAM buffers */
void saveDance(void);

struct move {
	uint16_t date; // in 0.1s
	uint16_t x; // in cm
	uint16_t y; // in cm
	uint8_t angle; // 256 would be 360deg or 2PI rad
	uint16_t startRadius; // in cm
	uint16_t endRadius; // in cm
};

struct color {
	uint16_t date; // in 0.1s
	uint8_t h;
	uint8_t s;
	uint8_t v;
	uint8_t fadeTime; // in 0.1s
};

/* pointers to current step in the dance
 * WARNING : read only, omitted const to avoid compiler's optimisation */
extern struct move *currentMove;
extern struct color *currentColor;

/* start sequencer thread */
void initSequencer(void);

#endif
