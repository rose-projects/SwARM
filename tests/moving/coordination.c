#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"
#include <math.h>
#include "hal.h"
#include "ch.h"
// To get the sign of a variable, returns 1 if positive or null, -1 if negative
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int xb = 200;
volatile int yb = 200;
volatile int distance = 0;
volatile int angle = 0;
volatile int dist_goal = 0;
volatile int angle_goal = 0;
volatile double orientation = 0;
volatile int x_pos = 0;
volatile int y_pos = 0;
volatile int last_angle_error = 0;
volatile int last_dist_error = 0;

static double radius;
static double alpha;
static int i;

void update_main_coordinates(){
    // Theta is the angle between Y axis and the orientation of the robot
    double theta = M_PI/2 - orientation;
    double xb_p;
    double yb_p;    

	/*
	 * Calculating coordinates of the next position to go to in the referential
	 * defined by the robot itself
	 */
	xb_p = (xb - x_pos)*cos(theta) - (yb-y_pos)*sin(theta);
	yb_p = (xb - x_pos)*sin(theta) + (yb-y_pos)*cos(theta);
	// Are we going forward? To the left?
	forward = SIGN(yb_p);
	to_the_left = SIGN(xb_p);

	// Calculating radius of the trajectory
	radius = (fabs(xb_p) + yb_p*yb_p/fabs(xb_p))/2;
	// Calculating angle of the trajectory
	alpha = 2*asin(sqrt((xb_p*xb_p)+(yb_p*yb_p))/(2*radius));
    
    i = 1;
}

void update_sub_coordinates(){
    // Updating distance and angle goals
    dist_goal = forward*fabs(alpha)*radius/N_POINTS;
    angle_goal = L_MM*forward*fabs(alpha)*radius/(N_POINTS*(radius*U_MM)*to_the_left);
    // Adding distance to previous goal
    dist_goal += last_dist_error;
    angle_goal += last_angle_error;

    // Ready for next iteration
    i++;
}
