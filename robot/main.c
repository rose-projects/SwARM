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
            // Capture ticks to start new phase of the movement
            tick_l_capt = tick_l;
            tick_r_capt = tick_r;

            angle_goal = 0;
            dist_goal = 0;

            printf("kieme tour %d\r\n", k);
            // 15cm straight line
            for(i = 0; i<17; i++){
                dist_goal += 40;
                printf("i ieme tour %d\r\n", i);
                printf("dist_goal %d\r\n", dist_goal);
                printf("tick_l %d\r\n", tick_l);
                printf("tick_r %d\r\n", tick_r);
                chThdSleepMilliseconds(50);
            }
            dist_goal += 10;
            chThdSleepMilliseconds(50);
            /*
               printf("Now waiting for robot to finish moving\r\n");
               chThdSleepMilliseconds(1000);
               printf("cmd_left %d\r\n", cmd_left);
               printf("cmd_right %d\r\n", cmd_right);
               printf("tick_l %d\r\n", tick_l);
               printf("tick_r %d\r\n", tick_r);
               */

            // 90 degrees angle
            for(j=0; j<11; j++){
                angle_goal -= 21;
                printf("j ieme tour %d\r\n", j);
                printf("angle_goal %d\r\n", angle_goal);
                printf("tick_l %d\r\n", tick_l);
                printf("tick_r %d\r\n", tick_r);
                chThdSleepMilliseconds(50);
            }
                angle_goal -= 19;
                chThdSleepMilliseconds(50);
            /*
               begin_new_pid();
               printf("Now waiting for robot to finish moving\r\n");
               printf("cmd_left %d\r\n", cmd_left);
               printf("cmd_right %d\r\n", cmd_right);
               printf("tick_l %d\r\n", tick_l);
               printf("tick_r %d\r\n", tick_r);
               */ 
    }
    printf("tick_l %d\r\n", tick_l);
    printf("tick_r %d\r\n", tick_r);

    while(1){
        chThdSleepMilliseconds(500);
    }
}
