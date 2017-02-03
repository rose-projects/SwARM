#include "ch.h"
#include "hal.h"
#include "RTT/SEGGER_RTT.h"
#include "RTT/RTT_streams.h"
#include "shell.h"
#include "chprintf.h"

#include <stdlib.h>

#include "exticonf.h"
#include "compdriver.h"
#include "pwmdriver.h"
#include "motors.h"
#include "asser.h"
#include "moving.h"
#include "coding_wheels.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2304)

volatile int cmd_left = 0;
volatile int cmd_right = 0;

static RTTStream rttStream;  

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

static void cmd_forward(BaseSequentialStream *chp, int argc, char *argv[]) {

	if ((argc == 0) || (argc > 1)){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		if(atoi(argv[0]) == 0){
			GO_REVERSE();
		} else{
			GO_FORWARD(); 
		}
	}
	return;
}

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

static void cmd_asser_angle(BaseSequentialStream *chp, int argc, char *argv[]) {

	(void) argv;

	if (argc > 0){
		chprintf(chp,"Usage: <command>\r\n");
		return;
	} else{
		chprintf(chp, "Testing the asser on the wheels\r\n");
		angle_goal = 492;
	}
	return;
}

static const ShellCommand commands[] = {
	{"l", cmd_mtr_l_p},
	{"r", cmd_mtr_r_p},
	{"s", cmd_status},
	{"f", cmd_forward},
	{"tt", cmd_test_ticks},
	{"aa", cmd_asser_angle},
	{NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
	(BaseSequentialStream *) &rttStream,
	commands
};

int main(void) {
	thread_t * shelltp = NULL;
	halInit();
	chSysInit();

	initExti();
	initComparators();
	initMotors();
	SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
	RTTObjectInit(&rttStream, 0);
	palSetLine(LINE_MTR_LED_R);
	palSetLine(LINE_MTR_LED_L);

	up(0);
	up(1);

	shellInit();

	chThdSleepMilliseconds(2000);

	while (true){
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
		chThdSleepMilliseconds(500);
	}
}
