#ifndef PID_H
#define PID_H

void initPID(void);
void beginNewPID(void);

extern volatile int distGoal;
extern volatile int angleGoal;

#endif
