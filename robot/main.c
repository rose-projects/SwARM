#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "adcconf.h"
#include "pwmdriver.h"
#include "pid.h"
#include "coding_wheels.h"
#include "swarmShell.h"

volatile int cmd_left = 0;
volatile int cmd_right = 0;

int main(void) {
    // initialize ChibiOS
    halInit();
    chSysInit();

    // initialize hardware
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    initPWM();
    initExti();
    initADC();
    initPID();

    chThdSleepMilliseconds(2000);


    int k;
    int i;
    int j;
    for(k = 0; k<4; k++){
            printf("kieme tour%d\r\n", k);
            // 15cm straight line
            for(i = 0; i<5; i++){
                printf("iieme tour%d\r\n", i);
                printf("cmd_left %d\r\n", cmd_left);
                printf("cmd_right %d\r\n", cmd_right);
                printf("dist_goal %d\r\n", dist_goal);
                printf("tick_l %d\r\n", tick_l);
                printf("tick_r %d\r\n", tick_r);
                dist_goal += 41;
                chThdSleepMilliseconds(50);
            }
            chThdSleepMilliseconds(200);
            printf("tick_l %d\r\n", tick_l);
            printf("tick_r %d\r\n", tick_r);

            // 90 degrees angle
            for(j=0; j<10; j++){
                printf("jieme tour%d\r\n", j);
                angle_goal += 25;
                chThdSleepMilliseconds(60);
            }
    }
    printf("tick_l %d\r\n", tick_l);
    printf("tick_r %d\r\n", tick_r);

    while(1){
        chThdSleepMilliseconds(500);
    }
}
