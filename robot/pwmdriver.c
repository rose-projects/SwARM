#include "ch.h"
#include "pwmdriver.h"

// approximative frequency of the PWM (in kHz)
#define PWM_FREQUENCY 60

void initPWM(void) {
	RCC->APB2ENR |= 0x00030000; // enable clocks for TIM15 and TIM16

	TIM15->CCER = 0x00; // disable capture/compare during setup
	TIM15->CR2 = 0x00;
	TIM15->BDTR = 0x8C00; // enable outputs
	TIM15->DIER = 0x00; // disable DMA and interrupts
	TIM15->SMCR = 0x00; // master mode
	TIM15->CCMR1 = 0x6800; // PWM mode 1
	TIM15->ARR = PWM_MAX;
	TIM15->PSC = 72000/(PWM_FREQUENCY*PWM_MAX) - 1; // setup prescaler
	TIM15->CCR2 = 0;
	TIM15->CCER = 0x10; // enable CC2
	TIM15->EGR = 0x01; // generate an update event to load the setup values
	TIM15->CR1 = 0x81; // enable counter

	TIM16->CCER = 0x00; // disable capture/compare during setup
	TIM16->CR2 = 0x00;
	TIM16->BDTR = 0x8C00; // enable outputs
	TIM16->DIER = 0x00; // disable DMA and interrupts
	TIM16->CCMR1 = 0x68; // PWM mode 1
	TIM16->ARR = PWM_MAX;
	TIM16->PSC = 72000/(PWM_FREQUENCY*PWM_MAX) - 1; // setup prescaler
	TIM16->CCR1 = 0;
	TIM16->CCER = 0x01; // enable CC1
	TIM16->EGR = 0x01; // generate an update event to load the setup values
	TIM16->CR1 = 0x81; // enable counter
}
