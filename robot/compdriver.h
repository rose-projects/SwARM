#ifndef COMPDRIVER_H
#define COMPDRIVER_H

#include "ch.h"

/* setup comparators 2 and 4 with DAC1 connectd to IN-, and setup DAC1 */
void initComparators(void);

/* set DAC1 output value (from 0 to 255) */
#define setDAC1value(val) do {DAC->DHR8R1 = val;} while(0)

/* to read COMP2 output value */
#define comp2out (COMP2->CSR & (1<<30))

#endif
