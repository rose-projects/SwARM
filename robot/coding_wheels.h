#ifndef CODING_WHEELS_H
#define CODING_WHEELS_H

extern volatile unsigned int tick_l;
extern volatile unsigned int tick_r;

// This file defines the constants that are used throughout the code and that 
// are related to the coding_wheels

// Number of ticks in a complete wheel rotation
#define TICKS_PER_ROTATION      (208)
// Displacement due to a complete wheel rotation
#define WHEEL_ROTATION_MM       (2*3.1415926535*R_MM)
// Unitary move, equivalent to one tick on both wheels ~= 0.725mm
#define U_MM                    (WHEEL_ROTATION_MM/TICKS_PER_ROTATION)
// Equivalent to a one tick difference in the rotation of the two wheels in rad
#define U_RAD                   (L_MM/U_MM)
// Equivalent to a one tick difference in the rotation of the two wheels
// ~= 0.361 or 997 ticks for a complete 360
#define U_DEGREE_ANGLE          (360*R_MM/(L_MM*TICKS_PER_ROTATION))
// Distance between the two wheels, width of the robots
#define L_MM                    (115)
// Wheel's radius
#define R_MM                    (24)
// Degrees angle to radian
#define DEG_TO_RAD              (3.1415926535/180)

#endif // CODING_WHEELS_H
