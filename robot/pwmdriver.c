#include "ch.h"
#include "hal.h"
#include "pwmdriver.h"

// approximative frequency of the PWM (in kHz)
static const unsigned int PWM_FREQUENCY_KHZ = 60;
static const unsigned int CLK_KHZ = 72000;

void initPWM(void) {
	RCC->APB2ENR |= 0x00030000; // enable clocks for TIM15 and TIM16
	RCC->APB1ENR |= 0x00000001; // enable clock for TIM2

	TIM15->CCER  = 0x00;   // disable capture/compare during setup
	TIM15->CR2   = 0x00;
	TIM15->BDTR  = 0x8C00; // enable outputs
	TIM15->DIER  = 0x00;   // disable DMA and interrupts
	TIM15->SMCR  = 0x00;   // master mode
	TIM15->CCMR1 = 0x6868; // PWM mode 1 on channels 1 and 2
	TIM15->ARR   = PWM_MAX - 1;
	TIM15->PSC   = CLK_KHZ/(PWM_FREQUENCY_KHZ * PWM_MAX) - 1; // setup prescaler
	TIM15->CCR1  = 0;
	TIM15->CCR2  = 0;
	TIM15->CCER  = 0x13;   // enable CC2 and CC1
	TIM15->EGR   = 0x01;   // generate an update event to load the setup values
	TIM15->CR1   = 0x81;   // enable counter

	TIM16->CCER  = 0x00;   // disable capture/compare during setup
	TIM16->CR2   = 0x00;
	TIM16->BDTR  = 0x8C00; // enable outputs
	TIM16->DIER  = 0x00;   // disable DMA and interrupts
	TIM16->CCMR1 = 0x68;   // PWM mode 1 on channel 1
	TIM16->ARR   = PWM_MAX - 1;
	TIM16->PSC   = CLK_KHZ/(PWM_FREQUENCY_KHZ * PWM_MAX) - 1; // setup prescaler
	TIM16->CCR1  = 0;
	TIM16->CCER  = 0x01;   // enable CC1
	TIM16->CNT   = 0;
	TIM16->EGR   = 0x01;   // generate an update event to load the setup values
	TIM16->CR1   = 0x80;

	TIM2->CCER  = 0x00;   // disable capture/compare during setup
	TIM2->CR2   = 0x00;
	TIM2->BDTR  = 0x8C00; // enable outputs
	TIM2->DIER  = 0x00;   // disable DMA and interrupts
	TIM2->CCMR1 = 0x68;   // PWM mode 1 on channel 1
	TIM2->ARR   = PWM_MAX - 1;
	TIM2->PSC   = CLK_KHZ/(PWM_FREQUENCY_KHZ * PWM_MAX) - 1; // setup prescaler
	TIM2->CCR1  = 0;
	TIM2->CCER  = 0x03;   // enable CC1
	TIM2->EGR   = 0x01;   // generate an update event to load the setup values
	TIM2->CNT   = 0;
	TIM2->CR1   = 0x81;   // enable counter

	TIM16->CR1   = 0x81;   // enable TIM16 counter
}
