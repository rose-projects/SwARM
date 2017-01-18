#include "asser.h"
#include "position.h"
#include "wheel_constants.h"
#include "coordination.h"

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the right
 */
void update_position(){
    // Calculating current angle value in radians
    float angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

    // Updating orientation
    orientation += angle_rad;
    // Calculating last coordinates of the robot
    x_pos += distance*cos(orientation);
    y_pos += distance*sin(orientation);
    last_dist_error = dist_error;
    last_angle_error = angle_error;

    chprintf(COUT, "Position update:\r\n");
    chprintf(COUT, "x_pos: %D\r\n", (long)x_pos);
    chprintf(COUT, "y_pos: %D\r\n", (long)y_pos);
    chprintf(COUT, "last_dist_error: %D\r\n", last_dist_error);
    chprintf(COUT, "last_angle_error: %D\r\n", last_angle_error);
}
