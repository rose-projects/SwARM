#include "ch.h"
#include "hal.h"

#include "RTT/SEGGER_RTT.h"
#include "exticonf.h"
#include "adcconf.h"
#include "pwmdriver.h"
#include "asser.h"
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
    initAsser();
    //initSwarmShell();

    chThdSleepMilliseconds(2000);


    int i;
    for(i = 0; i<28; i++){
        //swarmShellLife();
        chThdSleepMilliseconds(50);
        dist_goal += 50;
        i++;
        printf("cmd_left %d\r\n", cmd_left);
        printf("cmd_right %d\r\n", cmd_right);
    }
    while(1)
        chThdSleepMilliseconds(500);
}
