#ifndef PWMDRIVER_H
#define PWMDRIVER_H

/* setup TIM15 CH2 and TIM16 CH1 for PWM output */
void initPWM(void);

/* PWM maximum value, setting this value in the PWM channels will result in
 * a permanently high output */
#define PWM_MAX 200

/* set left motor PWM duty cycle (from 0 to PWM_MAX) */
#define setLpwm(val) do { TIM15->CCR2 = val; } while(0)

/* set right motor PWM duty cycle (from 0 to PWM_MAX) */
#define setRpwm(val) do { TIM16->CCR1 = val; } while(0)

#endif
