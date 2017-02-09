#ifndef IMU_H
#define IMU_H

/* currently measured angle in rad */
extern volatile float azimuth;

/* initialize magnetometer and start thread doing measurments at 100Hz */
int initIMU(void);

/* copy calibration data into RAM buffers */
void saveIMUcalibration(void);

/* write IMU calibration data in flash */
void writeIMUcalibration(void);

/* set the difference between north and (O, x) axis */
void setAzimuthDiff(float firstOrientation);

/* get ponderated azimuth */
float getAzimuth(void);

#endif
