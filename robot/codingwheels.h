#ifndef CODING_WHEELS_H
#define CODING_WHEELS_H

#include <math.h>

extern volatile int tickL;
extern volatile int tickR;

/*
 * This file defines the constants that are used throughout the code and that
 * are related to the coding_wheels
 */

// Number of ticks in a complete wheel rotation
#define TICKS_PER_ROTATION      208
// Distance between the two wheels, width of the robots
#define WHEEL_DIST              12.2f
// Wheels radius in cm
#define WHEEL_RADIUS_CM         2.4f

// TICKS/CM conversion constants
#define CM_TO_TICKS (TICKS_PER_ROTATION/(2*M_PI*WHEEL_RADIUS_CM))
#define TICKS_TO_CM ((2*M_PI*WHEEL_RADIUS_CM)/TICKS_PER_ROTATION)

// ticks difference between the two wheels to radian
#define TICKS_TO_RAD            (TICKS_TO_CM/WHEEL_DIST)

#endif
