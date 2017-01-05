#ifndef WHEELS_CONSTANTS_H
#define WHEELS_CONSTANTS_H

/* This file defines the constants that are used throughout the code and that 
 * are related to the coding_wheels
 */

// Pi value
#define PI                      (3.14159265359)
// Frequency of the timer of the input capture
#define FREQUENCY               (100000)
// Number of ticks in a complete wheel rotation
#define TICKS_PER_ROTATION      (103)
// Displacement due to a complete wheel rotation
#define WHEEL_ROTATION_MM       (2*PI*R_MM) 
// Unitary move, equivalent to one tick on both wheels
#define U_MM                    (WHEEL_ROTATION_MM/TICKS_PER_ROTATION)
// Unitary rotation, equivalent to a difference of one tick in the rotation of 
// the two wheels
#define U_DEGREE_ANGLE          (360*U_MM/(2*PI*L_MM))
// Distance between the two wheels, width of the robots
#define L_MM                    (115)
// Wheel's radius
#define R_MM                    (24)
// Degrees angle to radian
#define DEG_TO_RAD              (PI/180)

#endif
