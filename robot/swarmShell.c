#include "swarmShell.h"
#include "ch.h"
#include "RTT/SEGGER_RTT.h"
#include "RTT/RTT_streams.h"
#include "shell.h"
#include "chprintf.h"
#include <stdlib.h>
#include "asser.h"
#include "pwmdriver.h"
#include "coding_wheels.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2304)

static RTTStream rttStream;  

// This command sets left motor pwm
// Ex: l 100 
// Ex: l -100
static void cmd_mtr_l_p(BaseSequentialStream *chp, int argc, char *argv[]) {

	if ((argc == 0) || (argc > 1)){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		cmd_left = atoi(argv[0]);
		cmd_left = (cmd_left <= 200) ? cmd_left : 200;
		setLpwm(cmd_left);
		chprintf(chp,"cmd_left %d\r\n", cmd_left);

	}
	return;
}

// This command sets right motor pwm
// Ex: r 100 
// Ex: r -100
static void cmd_mtr_r_p(BaseSequentialStream *chp, int argc, char *argv[]) {

	if ((argc == 0) || (argc > 1)){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		cmd_right = atoi(argv[0]);
		cmd_right = (cmd_right <= 200) ? cmd_right : 200;
		setRpwm(cmd_right);
		chprintf(chp,"cmd_right %d\r\n", cmd_right);

	}
	return;
}

// This command prints out the status of the robot regarding the motors and the
// wheels
// Ex: s
static void cmd_status(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void) argv;

	if (argc > 0){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		chprintf(chp,"tick_l %d\r\n", tick_l);
		chprintf(chp,"tick_r %d\r\n", tick_r);
		chprintf(chp,"cmd_left %d\r\n", cmd_left);
		chprintf(chp,"cmd_right %d\r\n", cmd_right);
	}
	return;
}

// This command prints out the medium value of ticks per second, calculated
// in 10 seconds
// Ex: tt
static void cmd_test_ticks(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void) argv;

	if (argc > 0){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		chprintf(chp, "Testing the counter of the coding wheels\r\n");
		int tick_left = tick_l;
		int tick_right = tick_r;
		chThdSleepMilliseconds(10000);
		int tick_left_1 = (tick_l - tick_left)/10;
		int tick_right_1 = (tick_r - tick_right)/10;
		chprintf(chp, "tick_l freq Hz: %d\r\n", tick_left_1); 
		chprintf(chp, "tick_r freq Hz: %d\r\n", tick_right_1); 

	}
	return;
}


static const ShellCommand commands[] = {
	{"l", cmd_mtr_l_p},
	{"r", cmd_mtr_r_p},
	{"s", cmd_status},
	{"tt", cmd_test_ticks},
	{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
	(BaseSequentialStream *) &rttStream,
	commands
};

void swarmShellLife(void){
    static thread_t * shelltp = NULL;

    if (!shelltp) {
        // Spawns a new shell
        shelltp = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO-1);
    } else {
        // If the previous shell exited
        if (chThdTerminatedX(shelltp)) {
            // Recovers memory of the previous shell
            chThdRelease(shelltp);
            shelltp = NULL;
        }
    }
}

void initSwarmShell(void){
	RTTObjectInit(&rttStream, 0);
    shellInit();
}
