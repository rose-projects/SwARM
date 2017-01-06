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
 * The calculation depends on wether we are rotating to the left or to the rigrt
 */
void update_position(){
    float angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

    if(to_the_left){
        orientation += angle_rad;
        x_pos += distance*cos(orientation);
        y_pos += distance*sin(orientation);
    }
    else{
        orientation -= angle_rad;
        x_pos += distance*cos(orientation);
        y_pos += distance*sin(orientation);
    }

    chprintf(COUT, "Distance update:\r\n");
    chprintf(COUT, "distance: %D\r\n", distance);
    chprintf(COUT, "x_pos: %D\r\n", (long)x_pos);
    chprintf(COUT, "y_pos: %D\r\n", (long)y_pos);
    chprintf(COUT, "orientation: %D\r\n", (long)orientation);
}
