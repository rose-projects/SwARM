#include "ch.h"
#include "hal.h"

#include "position.h"
#include "wheel_constants.h"
#include "asser.h"
#include "coordination.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the right
 */
void update_position(){
    // Calculating current angle value in radians
    double angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

    // Updating orientation
    orientation += angle_rad;
    // Calculating last coordinates of the robot
    x_pos += distance*cos(orientation);
    y_pos += distance*sin(orientation);
    last_dist_error = dist_error;
    last_angle_error = angle_error;
}
