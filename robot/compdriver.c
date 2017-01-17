#include "ch.h"

void initComparators(void) {
	RCC->APB1ENR |= 0x20000000; // enable DAC1 clock
	DAC->CR = 0x01; // enable DAC 1
	DAC->DHR8R1 = 0x80; // by default, DAC1 outputs Vdd/2

	RCC->APB2ENR |= 0x01; // enable SYSCFG clock (needed by comparators)
	COMP2->CSR = 0x00000041; // enable comparator 2, IN- from DAC1
	COMP4->CSR = 0x00000041; // enable comparator 4, IN- from DAC1
}
