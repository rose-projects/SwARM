#ifndef COMPDRIVER_H
#define COMPDRIVER_H

#include "ch.h"

/* setup comparators 2 and 4 with DAC1 connectd to IN-, and setup DAC1 */
void initComparators(void);

// Sets left or right wheel comp ref to 1/4 of Vrefint
void up(int right);
// Sets left or right wheel comp ref to 3/4 of Vrefint
void down(int right);

extern volatile int up_l;
extern volatile int up_r;

/* set DAC1 output value (from 0 to 255) */
#define setDAC1value(val) do {DAC->DHR8R1 = val;} while(0)

/* to read COMP2 output value */
#define comp2out (COMP2->CSR & (1<<30))

#endif
