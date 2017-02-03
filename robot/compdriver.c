#include "compdriver.h"
#include "ch.h"

#define MASK(right) (1 << ((right) ? 30 : 22))
#define COMPCSR(right) ((right) ? &COMP2->CSR : &COMP4->CSR)
#define UP(right) ((right) ? &up_r : &up_l)

volatile int up_l = 0;
volatile int up_r = 0;

void initComparators(void) {
	RCC->APB2ENR |= 0x01; // enable SYSCFG clock (needed by comparators)
	RCC->APB1ENR |= 0x20000000; // enable DAC1 clock
	DAC->CR = 0x01; // enable DAC 1
	DAC->DHR8R1 = 0xC0; // by default, DAC1 outputs Vrefint*3/8
}

void up(int right){
	*UP(right) = 1;
	*COMPCSR(right) = 0x11;
	EXTI->RTSR |= MASK(right);
	EXTI->FTSR &= ~MASK(right);
	EXTI->PR = MASK(right);
}

void down(int right){
	*UP(right) = 0;
	*COMPCSR(right) = 0x41;
	EXTI->FTSR |= MASK(right);
	EXTI->RTSR &= ~MASK(right);
	EXTI->PR = MASK(right);
}
