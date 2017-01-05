#include "position.h"
#include "wheel_constants.h"

#include "ch.h"
#include "hal.h"
#include <math.h>
#include "asser.h"

/*
 * Update position according to the coding wheels
 * The calculation depends on wether we are rotating to the left or to the rigrt
 */
void update_position(){
    float angle_rad = angle*U_DEGREE_ANGLE*DEG_TO_RAD;

    if(to_the_left){
        x_pos += distance*cos(orientation + angle_rad);
        y_pos += distance*sin(orientation + angle_rad);
    }
    else{
        x_pos += distance*cos(orientation - angle_rad);
        y_pos += distance*sin(orientation - angle_rad);
    }
}
