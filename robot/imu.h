#ifndef IMU_H
#define IMU_H

// hard coded angle diff(x, north)
#define X_NORTH_DIFF (- PI/4) // north around -y

extern float azimuth;
int initIMU(void);

#endif