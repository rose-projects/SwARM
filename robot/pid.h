#ifndef PID_H
#define PID_H

void initPID(void);
void begin_new_pid(void);

extern volatile int dist_goal;
extern volatile int angle_goal;
extern volatile int dist_error;
extern volatile int angle_error;
extern volatile int cmd_left;
extern volatile int cmd_right;
extern volatile int tick_l_capt;
extern volatile int tick_r_capt;

#endif
