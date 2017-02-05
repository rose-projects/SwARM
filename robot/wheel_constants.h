#ifndef WHEELS_CONSTANTS_H
#define WHEELS_CONSTANTS_H

#include <math.h>

/* 
 * This file defines the constants that are used throughout the code and that 
 * are related to the coding_wheels
 */

// Number of ticks in a complete wheel rotation
#define TICKS_PER_ROTATION      (208)
// Displacement due to a complete wheel rotation
#define WHEEL_ROTATION_MM       (2*M_PI*R_MM)
// Unitary move, equivalent to one tick on both wheels
#define U_MM                    (WHEEL_ROTATION_MM/TICKS_PER_ROTATION) // ~= 0.725mm
// Equivalent to a one tick difference in the rotation of the two wheels in rad
#define U_RAD                   ((M_PI*M_PI*L_MM)/(180*U_MM))
// Equivalent to a one tick difference in the rotation of the two wheels
#define U_DEGREE_ANGLE          (360*R_MM/(L_MM*TICKS_PER_ROTATION)) // ~= 0.361 or 997 ticks for a complete 360
// Distance between the two wheels, width of the robots
#define L_MM                    (115)
// Wheel's radius
#define R_MM                    (24)
// Degrees angle to radian
#define DEG_TO_RAD              (M_PI/180)

#endif
