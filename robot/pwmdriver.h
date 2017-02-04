#ifndef PWMDRIVER_H
#define PWMDRIVER_H

/* setup TIM15 CH2 and TIM16 CH1 for PWM output */
void initPWM(void);

/* PWM maximum value, setting this value in the PWM channels will result in
 * a permanently high output */
#define PWM_MAX 400

/* set left motor PWM duty cycle (from -PWM_MAX/2 to PWM_MAX/2) */
#define setLpwm(val) do { TIM15->CCR1 = val + PWM_MAX/2; TIM15->CCR2 = val + PWM_MAX/2; } while(0)

/* set right motor PWM duty cycle (from -PWM_MAX/2 to PWM_MAX/2) */
#define setRpwm(val) do { TIM16->CCR1 = val + PWM_MAX/2; TIM2->CCR1 = val + PWM_MAX/2; } while(0)

#endif
