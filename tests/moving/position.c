#include "asser.h"
#include "position.h"
#include "wheel_constants.h"

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "chprintf.h"
#include <math.h>

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

    chprintf(COUT, "Position update:\r\n");
    chprintf(COUT, "x_pos: %D\r\n", (long)x_pos);
    chprintf(COUT, "y_pos: %D\r\n", (long)y_pos);
}
