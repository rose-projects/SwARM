#ifndef POSITION_H
#define POSITION_H

extern int tickLprev, tickRprev;
extern volatile float currentX, currentY;

void updatePosition(float *currentOrientation);
void initFusion(void);

#endif // POSITION_H
