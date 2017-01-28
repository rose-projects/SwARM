#include "asser.h"
#include "position.h"
#include "wheel_constants.h"
#include "coordination.h"
#include "coding_wheels.h"

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the right
 */
void update_position(){
    static unsigned int tick_l_prev = 0; // last tick count for left wheel
    static unsigned int tick_r_prev = 0; // last tick conut for right wheel
    unsigned int tick_l_current = tick_l; // current tick count for left wheel
    unsigned int tick_r_current = tick_r; // current tick count for right wheel

    // Calculating the distance the robot moved in the last 50ms
    int distance_current = (tick_l_current + tick_r_current \
            - tick_l_prev - tick_r_prev)/2;
    // Calculating the angle the robot turned in the last 50ms
    int angle_current = ((tick_l_current - tick_l_prev) \
            - (tick_r_current - tick_r_prev));


    // Preparing next call of the function
    tick_l_prev = tick_l_current;
    tick_r_prev = tick_r_current;

    // Calculating current angle value in radians
    double angle_rad = angle_current*U_DEGREE_ANGLE*DEG_TO_RAD;

    // Updating orientation
    orientation += angle_rad;
    // Calculating last coordinates of the robot
    x_pos += distance_current*cos(orientation);
    y_pos += distance_current*sin(orientation);

    // Updating dist and angle errors that must be covered 
    last_dist_error = dist_error;
    last_angle_error = angle_error;

    chprintf(COUT, "Position update:\r\n");
    chprintf(COUT, "x_pos: %D\r\n", (long)x_pos);
    chprintf(COUT, "y_pos: %D\r\n", (long)y_pos);
    chprintf(COUT, "last_dist_error: %D\r\n", last_dist_error);
    chprintf(COUT, "last_angle_error: %D\r\n", last_angle_error);
}
