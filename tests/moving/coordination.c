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

volatile int dist_goal = 0; /* PID->dist to next update_sub_coordinates call */
volatile int angle_goal = 0; /* PID->tick diff: 0 is straigt, 246 is Pi/2 */
volatile double orientation = 0; /* orientation of the robot in rad */
volatile int x_pos = 0; /* last measured position, used when calling update_ */
volatile int y_pos = 0; /* main_coordinates */
volatile int last_angle_error = 0; /* computed in position.c */
volatile int last_dist_error = 0; /* computed in position.c */

static double radius; /* radius of the trajectory */
static double alpha; /* angle of the trajectory */
static int i; /* i-th point out of N_POINTS in the trajectory */
static int forward; /* 1: going forward, -1: backward */
static int to_the_left; /* 1: going to the left, -1 to the right */
static double x_goal_, y_goal_, goal_angle_; /* variables in the new system */

/* Called once to set the goalination */
void update_main_coordinates(int x_goal, int y_goal, double goal_angle,
	int r_dep, int r_goal)
{
	double theta = M_PI/2 - orientation; /* system rotation angle */
	int dep_circle[2] = {0};
	int goal_circle[2] = {0};
	int tmp1[2], tmp2[2]; /* the 2 arrival circles we have to chose from */

	/* testing order */
	x_goal = 200;
	y_goal = 200;
	goal_angle = M_PI / 4;
	r_dep = 10;
	r_goal = 40;

	/* cartesian system change */
	x_goal_ = (x_goal-x_pos)*cos(theta) - (y_goal-y_pos)*sin(theta);
	y_goal_ = (x_goal-x_pos)*sin(theta) + (y_goal-y_pos)*cos(theta);
	goal_angle_ = goal_angle + theta;

	/* choose the right circles */
	dep_circle[0] = (x_goal_ >= 0 ? r_dep : -r_dep);
	tmp1[0] = x_goal_ + r_goal * sin(goal_angle_);
	tmp1[1] = y_goal_ - r_goal * cos(goal_angle_);
	tmp2[0] = x_goal_ - r_goal * sin(goal_angle_);
	tmp2[1] = y_goal_ + r_goal * cos(goal_angle_);
	if (tmp1[0]*tmp1[0] + tmp1[1]*tmp1[1]
		<= tmp2[0]*tmp2[0] + tmp2[1]*tmp2[1])
	{
		goal_circle[0] = tmp1[0];
		goal_circle[1] = tmp1[1];
	}

	/* find the tangent points */

	i = 1;
}

/* Update distance and angle goals
 * Called every 50ms
 */
void update_sub_coordinates(void) {
	forward = SIGN(y_goal_);
	to_the_left = SIGN(x_goal_);

	radius = (fabs(x_goal_) + y_goal_*y_goal_/fabs(x_goal_)) / 2;
	alpha = 2*asin(sqrt((x_goal_*x_goal_)+(y_goal_*y_goal_)) / (2*radius));

	dist_goal = forward*fabs(alpha)*radius*i/N_POINTS;
	angle_goal = L_MM*dist_goal/(radius*U_MM)*to_the_left;
	dist_goal += last_dist_error;
	angle_goal += last_angle_error;
	i++;
}

