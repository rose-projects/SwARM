#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"
#include <math.h>
#include "hal.h"
#include "ch.h"
/* return 1 if x >= 0, -1 otherwise */
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int dist_goal = 0; // PID->dist to next sub_coord call
volatile int angle_goal = 0; // PID->tick diff : 0 is straigt, 246 is Pi/2
volatile double orientation = 0; // robot's orientation in rad
volatile int x_pos = 0; // last measured position : true when exec main_coord
volatile int y_pos = 0; // ^
volatile int last_angle_error = 0; // PID
volatile int last_dist_error = 0; // ^

static double radius; /* radius of the trajectory */
static double alpha; /* angle of the trajectory */
static int i; // i-th point out of N_POINTS in the trajectory
static int forward;
static int to_the_left;

/* Called once to go from A to B */
void update_main_coordinates(int xb, int yb, double arrival_angle) {
	double xb_, yb_, arrival_angle_; // variables in the new system
	double theta = M_PI/2 - orientation; // system rotation angle	

	/* testing order */
	xb = 200;
	yb = 200;
	arrival_angle = M_PI / 4;

	xb_ = (xb-x_pos)*cos(theta) - (yb-y_pos)*sin(theta);
	yb_ = (xb-x_pos)*sin(theta) + (yb-y_pos)*cos(theta);
	arrival_angle_ = M_PI/2 - arrival_angle;

	// Are we going forward? To the left?
	forward = SIGN(yb_);
	to_the_left = SIGN(xb_);

	radius = (fabs(xb_) + yb_*yb_/fabs(xb_)) / 2;
	alpha = 2*asin(sqrt((xb_*xb_)+(yb_*yb_)) / (2*radius));
	i = 1;
    (void) arrival_angle_;
}

// Update distance and angle goals
// Called every 50ms
void update_sub_coordinates(void) {
	dist_goal = forward*fabs(alpha)*radius*i/N_POINTS;
	angle_goal = L_MM*dist_goal/(radius*U_MM)*to_the_left;
	dist_goal += last_dist_error;
	angle_goal += last_angle_error;
	i++;
}

