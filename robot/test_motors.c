#include "ch.h"

#include "test_motors.h"
#include "led.h"
#include "pwmdriver.h"

void test_motors(void){
    // Starting value for the PWMs
    unsigned int pwm_pwr = 200;
    // Time to sleep between two PWM values
    const unsigned int sleep_ms = 2000;
    // V value for HSV of the LEDs, it makes them toggle
    unsigned int v_hsv = 128;

    for(int i=0;i<20;i++){
        setLpwm(pwm_pwr);
        setRpwm(pwm_pwr);
        setColor(pwm_pwr, 255, v_hsv);
        pwm_pwr -= 10;
        v_hsv += 128;
        chThdSleepMilliseconds(sleep_ms);
    }
}
