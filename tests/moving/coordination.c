#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"

#include <math.h>

// return 1 if x >= 0, -1 otherwise
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int dist_goal = 0;        // PID->dist to next update_sub_coordinates call
volatile int angle_goal = 0;       // PID->tick diff: 0 is straigt, 246 is Pi/2
volatile double orientation = 0;   // orientation of the robot in rad
volatile int x_pos = 0;            // last measured position, used when calling update_
volatile int y_pos = 0;            // main_coordinates
volatile int last_angle_error = 0; // computed in position.c
volatile int last_dist_error = 0;  // computed in position.c

static int n_pts;                            // number of points before the dest
static double x_dest, y_dest, ori_dest;      // make the parameters global
static double r_dep, r_dest;                 // radius of the departure and destination circles
static double pt_tan_dep[2], pt_tan_dest[2]; // tangent points
static int is_inner_tan = 0;                 // 1 if inner tangent, -1 otherwise
static int state = 0;                        // 0 departure circles, 1 straight line, 2 dest circle
static int i;                                // i-th point out of n_pts in the trajectory
static int to_the_left;                      // 1: going to the left, -1 to the right

// Called once to set the destination
void update_main_coordinates(int x_dest_, int y_dest_, double ori_dest_,
                             int r_dep_, int r_dest_, int n_pts_)
{
	int dep_circle[2] = {0};
	int dest_circle[2] = {0};
	int tmp1[2], tmp2[2]; // the 2 circles we have to chose from
	double h[2] = {0};    // homothetic center

	// testing order
	x_dest_ = 200;
	y_dest_ = 200;
	ori_dest_ = 120;
	r_dep_ = 50;
	r_dest_ = 20;

	/* Because of the method of the homothetic center, both circles cannot
	 * be of the same radius, this dirty hack solves this issue.
	 */
	if (r_dep_ == r_dest_) {
		r_dep_++;
	}

	x_dest = x_dest_;
	y_dest = y_dest_;
	ori_dest = ori_dest_;
	r_dep = r_dep_;
	r_dest = r_dest_;
	n_pts = n_pts_;

	/* choose the right circles
	 * hypothesis: the closest to the destination is the one we want.
	 * Some cases break this law, but they may not happen depending on the
	 * choice of the choreography.
	 */
	tmp1[0] = x_pos + r_dep*sin(orientation);
	tmp1[1] = y_pos - r_dep*cos(orientation);
	tmp2[0] = x_pos - r_dep*sin(orientation);
	tmp2[1] = y_pos + r_dep*cos(orientation);
	if ((tmp1[0]-x_dest)*(tmp1[0]-x_dest) + (tmp1[1]-y_dest)*(tmp1[1]-y_dest) <=
	    (tmp2[0]-x_dest)*(tmp2[0]-x_dest) + (tmp2[1]-y_dest)*(tmp2[1]-y_dest))
	{
		dep_circle[0] = tmp1[0];
		dep_circle[1] = tmp1[1];
	} else {
		dep_circle[0] = tmp2[0];
		dep_circle[1] = tmp2[1];
	}

	tmp1[0] = x_dest + r_dest*sin(ori_dest);
	tmp1[1] = y_dest - r_dest*cos(ori_dest);
	tmp2[0] = x_dest - r_dest*sin(ori_dest);
	tmp2[1] = y_dest + r_dest*cos(ori_dest);
	if ((tmp1[0]-x_pos)*(tmp1[0]-x_pos) + (tmp1[1]-y_pos)*(tmp1[1]-y_pos) <=
	    (tmp2[0]-x_pos)*(tmp2[0]-x_pos) + (tmp2[1]-y_pos)*(tmp2[1]-y_pos))
	{
		dest_circle[0] = tmp1[0];
		dest_circle[1] = tmp1[1];
	} else {
		dest_circle[0] = tmp2[0];
		dest_circle[1] = tmp2[1];
	}

	/* find the tangent points
	 * hypothesis1: the robot will never need to to go to the left if
	 * the goal is in the right-hand quadrant
	 * hypothesis2: r_dep > r_dest TODO: all cases
	 */
	if (dest_circle[0] == tmp1[0]) {
		is_inner_tan = -1;
	} else {
		is_inner_tan = 1;
	}

	h[0] = (r_dep*dep_circle[0] + is_inner_tan*r_dest*dest_circle[0]) *
	       (r_dep + is_inner_tan*r_dest);
	h[1] = (r_dep*dep_circle[1] + is_inner_tan*r_dest*dest_circle[1]) *
	       (r_dep + is_inner_tan*r_dest);

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

	pt_tan_dest[0] = dest_circle[0] +
	                 (r_dest*r_dest*(h[0]-dest_circle[0]) +
	                  is_inner_tan*r_dest*(h[1]-dest_circle[0]) *
	                  sqrt((h[0]-dest_circle[0])*(h[0]-dest_circle[0]) +
	                       (h[1]-dest_circle[1])*(h[1]-dest_circle[1]) -
	                        r_dest*r_dest)) /
	                 ((h[0]-dest_circle[0])*(h[0]-dest_circle[0]) +
	                  (h[1]-dest_circle[1])*(h[1]-dest_circle[1]));
	pt_tan_dest[1] = dest_circle[1] +
	                 (r_dest*r_dest*(h[1]-dest_circle[1]) -
	                  is_inner_tan*r_dest*(h[0]-dest_circle[0]) *
	                  sqrt((h[0]-dest_circle[0])*(h[0]-dest_circle[0]) +
	                       (h[1]-dest_circle[1])*(h[1]-dest_circle[1]) -
	                        r_dest*r_dest)) /
	                 ((h[0]-dest_circle[0])*(h[0]-dest_circle[0]) +
	                  (h[1]-dest_circle[1])*(h[1]-dest_circle[1]));

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
		radius = r_dep;
		to_the_left = -SIGN(x);
		break;
	case 1:
		x = pt_tan_dest[0];
		y = pt_tan_dest[1];
		break;
	case 2:
		x = x_dest;
		y = y_dest;
		radius = r_dest;
		if (x_dest >= 0) {
			to_the_left = is_inner_tan;
		} else {
			to_the_left = -is_inner_tan;
		}
		break;
	}

	switch (state) {
	case 1:
		dist_goal = sqrt((pt_tan_dest[0]-pt_tan_dep[0]) *
		                 (pt_tan_dest[0]-pt_tan_dep[0])) *
		            i/n_pts;
		angle_goal = 0;
		break;
	case 0:
	case 2:
		alpha = 2*asin(sqrt((x*x)+(y*y)) / (2*radius));
		dist_goal += fabs(alpha)*radius/n_pts;
		angle_goal += -L_MM*dist_goal/(radius*U_MM)*to_the_left;
		break;
	}

	dist_goal += last_dist_error;
	angle_goal += last_angle_error;

	// TODO: use computed values to have a constant speed
	if (i == n_pts/3) {
		state = 1;
	} else if (i == 2*n_pts/3) {
		state = 2;
	}
	i++;
}

