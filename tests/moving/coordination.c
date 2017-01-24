#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"

#include <math.h>

#include "hal.h"
#include "ch.h"

// return 1 if x >= 0, -1 otherwise
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int dist_goal = 0;        // PID->dist to next update_sub_coordinates call
volatile int angle_goal = 0;       // PID->tick diff: 0 is straigt, 246 is Pi/2
volatile double orientation = 0;   // orientation of the robot in rad
volatile int x_pos = 0;            // last measured position, used when calling update_
volatile int y_pos = 0;            // main_coordinates
volatile int last_angle_error = 0; // computed in position.c
volatile int last_dist_error = 0;  // computed in position.c

static double r_dep_, r_goal_;               // radius of the departure and goal circles
static double x_goal_, y_goal_, goal_angle_; // variables in the new system
static double pt_tan_dep[2], pt_tan_goal[2]; // tangent points
static int t = 0;                            // 1 if inner tangent, -1 otherwise
static int state = 0;                        // 0 departure circles, 1 straight line, 2 goal circle
static int i;                                // i-th point out of N_POINTS in the trajectory
static int to_the_left;                      // 1: going to the left, -1 to the right

// Called once to set the goalination
void update_main_coordinates(int x_goal, int y_goal, double goal_angle,
	int r_dep, int r_goal)
{
	double theta = M_PI/2 - orientation; // system rotation angle
	int dep_circle[2] = {0};
	int goal_circle[2] = {0};
	int tmp1[2], tmp2[2]; // the 2 arrival circles we have to chose from
	double h[2] = {0};    // homothetic center

	// testing order
	x_goal = 200;
	y_goal = 200;
	goal_angle = 120;
	r_dep = 50;
	r_goal = 20;

	// Because of the method of the homothetic center, both circles cannot
	// be of the same radius, this dirty hack solves this issue.
	if (r_dep == r_goal) {
		r_dep++;
	}

	r_dep_ = r_dep;
	r_goal_ = r_goal;

	// cartesian system change
	x_goal_ = (x_goal-x_pos)*cos(theta) - (y_goal-y_pos)*sin(theta);
	y_goal_ = (x_goal-x_pos)*sin(theta) + (y_goal-y_pos)*cos(theta);
	goal_angle_ = goal_angle + theta;

	/* choose the right circles
	 * hypothesis: the closest to the goal is the one we want.
	 * Some cases break this law, but they may not happen depending on the
	 * choice of the choreography.
	 */

	dep_circle[0] = (x_goal_ >= 0 ? r_dep : -r_dep);
	tmp1[0] = x_goal_ + r_goal * sin(goal_angle_);
	tmp1[1] = y_goal_ - r_goal * cos(goal_angle_);
	tmp2[0] = x_goal_ - r_goal * sin(goal_angle_);
	tmp2[1] = y_goal_ + r_goal * cos(goal_angle_);
	if (tmp1[0]*tmp1[0] + tmp1[1]*tmp1[1] <= tmp2[0]*tmp2[0] + tmp2[1]*tmp2[1]) {
		goal_circle[0] = tmp1[0];
		goal_circle[1] = tmp1[1];
	}

	// find the tangent points
	/* hypothesis1: the robot will never need to to go to the left if
	 * the goal is in the right-hand quadrant
	 * hypothesis2: r_dep > r_goal TODO: all cases
	 */
	if (goal_circle[0] == tmp1[0]) {
		t = -1;
	} else {
		t = 1;
	}

	h[0] = (r_dep*dep_circle[0] + t*r_goal*goal_circle[0]) * (r_dep + t*r_goal);
	h[1] = (r_dep*dep_circle[1] + t*r_goal*goal_circle[1]) * (r_dep + t*r_goal);

	pt_tan_dep[0] = dep_circle[0] +
	                (r_dep*r_dep*(h[0]-dep_circle[0]) -
	                 r_dep*(h[1]-dep_circle[1]) *
	                 sqrt((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) +
	                      (h[1]-dep_circle[1])*(h[1]-dep_circle[1]) -
	                       r_dep*r_dep)) /
	                ((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) +
	                 (h[1]-dep_circle[1])*(h[1]-dep_circle[1]));
	pt_tan_dep[1] = dep_circle[1] +
	                (r_dep*r_dep*(h[1]-dep_circle[1]) +
	                 r_dep*(h[0]-dep_circle[0]) *
	                 sqrt((h[0]-dep_circle[0])*(h[0] -  dep_circle[0]) +
	                      (h[1]-dep_circle[1])*(h[1]-dep_circle[1]) -
	                      r_dep*r_dep)) /
	                ((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) +
	                 (h[1]-dep_circle[1])*(h[1]-dep_circle[1]));

	pt_tan_goal[0] = goal_circle[0] +
	                 (r_goal*r_goal*(h[0]-goal_circle[0]) +
	                  t*r_goal*(h[1]-goal_circle[0]) *
	                  sqrt((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) +
	                       (h[1]-goal_circle[1])*(h[1]-goal_circle[1]) -
	                        r_goal*r_goal)) /
	                 ((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) +
	                  (h[1]-goal_circle[1])*(h[1]-goal_circle[1]));
	pt_tan_goal[1] = goal_circle[1] +
	                 (r_goal*r_goal*(h[1]-goal_circle[1]) -
	                  t*r_goal*(h[0]-goal_circle[0]) *
	                  sqrt((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) +
	                       (h[1]-goal_circle[1])*(h[1]-goal_circle[1]) -
	                        r_goal*r_goal)) /
	                 ((h[0]-goal_circle[0])*(h[0]-goal_circle[0]) +
	                  (h[1]-goal_circle[1])*(h[1]-goal_circle[1]));

	state = 0;
	i = 1;
}

// Update distance and angle goals: called every 50ms
void update_sub_coordinates(void) {
	double x = 0, y = 0; // coordinate of the destination
	double radius = 0;   // radius of the trajectory
	double alpha = 0;    // angle of the trajectory

	switch (state) {
	case 0:
		x = pt_tan_dep[0];
		y = pt_tan_dep[1];
		radius = r_dep_;
		to_the_left = -SIGN(x);
		break;
	case 1:
		x = pt_tan_goal[0];
		y = pt_tan_goal[1];
		break;
	case 2:
		x = x_goal_;
		y = y_goal_;
		radius = r_goal_;
		if (x_goal_ >= 0) {
			to_the_left = t;
		} else {
			to_the_left = -t;
		}
		break;
	}

	switch (state) {
	case 1:
		dist_goal = sqrt((pt_tan_goal[0]-pt_tan_dep[0]) *
		                 (pt_tan_goal[0]-pt_tan_dep[0])) *
		            i/N_POINTS;
		angle_goal = 0;
		break;
	case 0:
	case 2:
		alpha = 2*asin(sqrt((x*x)+(y*y)) / (2*radius));
		dist_goal += fabs(alpha)*radius/N_POINTS;
		angle_goal += -L_MM*dist_goal/(radius*U_MM)*to_the_left;
		break;
	}

	dist_goal += last_dist_error;
	angle_goal += last_angle_error;

	if (i == 7) {
		state = 1;
	} else if (i == 14) {
		state = 2;
	}
	i++;
}

