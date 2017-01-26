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
volatile int last_angle_error = 0; // computed in position.c
volatile int last_dist_error = 0;  // computed in position.c

double orientation = 0;   // orientation of the robot in rad
int x_pos = 0;            // last measured position, used when calling update_
int y_pos = 0;            // main_coordinates

static int i;                                // i-th point out of n_pts in the trajectory
static int x_dep, y_dep, x_dest, y_dest;     // make the parameters global
static double ori_dest;
static double r_dep, r_dest;
static int n_pts;                            // number of intermediary points
static int dep_pts, straight_pts, dest_pts;
static int dep_circle[2], dest_circle[2];
static double pt_tan_dep[2], pt_tan_dest[2]; // tangent points
static int is_inner_tan = 0;                 // 1 if inner tangent, -1 otherwise
static int dep_to_left, dest_to_left;        // 1: going to the left, -1 to the right
static double angle_dep, angle_dest;
static double tot_len, dep_len, straight_len, dest_len;

// Called once to set the destination
void update_main_coordinates(int x_dest_, int y_dest_, double ori_dest_,
                             int r_dep_, int r_dest_, int n_pts_)
{
	int tmp1[2], tmp2[2]; // the 2 circles we have to chose from
	double h[2] = {0};    // homothetic center

	/* Because of the method of the homothetic center, both circles cannot
	 * be of the same radius, this dirty hack solves the issue.
	 */
	if (r_dep_ == r_dest_) {
		r_dep_++;
	}

	x_dep = x_pos;
	y_dep = y_pos;
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
	tmp1[0] = x_dep + r_dep*sin(orientation);
	tmp1[1] = y_dep - r_dep*cos(orientation);
	tmp2[0] = x_dep - r_dep*sin(orientation);
	tmp2[1] = y_dep + r_dep*cos(orientation);
	if ((tmp1[0]-x_dest)*(tmp1[0]-x_dest) + (tmp1[1]-y_dest)*(tmp1[1]-y_dest) <=
	    (tmp2[0]-x_dest)*(tmp2[0]-x_dest) + (tmp2[1]-y_dest)*(tmp2[1]-y_dest))
	{
		dep_to_left = -1;
		dep_circle[0] = tmp1[0];
		dep_circle[1] = tmp1[1];
	} else {
		dep_to_left = 1;
		dep_circle[0] = tmp2[0];
		dep_circle[1] = tmp2[1];
	}

	tmp1[0] = x_dest + r_dest*sin(ori_dest);
	tmp1[1] = y_dest - r_dest*cos(ori_dest);
	tmp2[0] = x_dest - r_dest*sin(ori_dest);
	tmp2[1] = y_dest + r_dest*cos(ori_dest);
	if ((tmp1[0]-x_dest)*(tmp1[0]-x_dest) + (tmp1[1]-y_dest)*(tmp1[1]-y_dest) <=
	    (tmp2[0]-x_dest)*(tmp2[0]-x_dest) + (tmp2[1]-y_dest)*(tmp2[1]-y_dest))
	{
		dept_to_left = -1;
		dest_circle[0] = tmp1[0];
		dest_circle[1] = tmp1[1];
	} else {
		dest_to_right = 1;
		dest_circle[0] = tmp2[0];
		dest_circle[1] = tmp2[1];
	}

	/* find the tangent points
	 * hypothesis1: the robot will never need to to go to the left if
	 * the goal is in the right-hand quadrant, nor behind it
	 * hypothesis2: r_dep > r_dest TODO: all cases
	 */
	if (dest_circle[0] == tmp1[0]) {
		is_inner_tan = -1;
	} else {
		is_inner_tan = 1;
	}

	h[0] = (r_dep*dest_circle[0] + is_inner_tan*r_dest*dep_circle[0]) /
	       (r_dep + is_inner_tan*r_dest);
	h[1] = (r_dep*dest_circle[1] + is_inner_tan*r_dest*dep_circle[1]) /
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
	  sqrt((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) +
	       (h[1]-dep_circle[1])*(h[1]-dep_circle[1]) -
	       r_dep*r_dep)) /
	 ((h[0]-dep_circle[0])*(h[0]-dep_circle[0]) +
	  (h[1]-dep_circle[1])*(h[1]-dep_circle[1]));

	pt_tan_dest[0] = dest_circle[0] +
	 (r_dest*r_dest*(h[0]-dest_circle[0]) +
	  is_inner_tan*r_dest*(h[1]-dest_circle[1]) *
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

	angle_dep = acos(
	 (((dep_circle[0]-x_dep)*(dep_circle[0]-x_dep)+
	   (dep_circle[1]-y_dep)*(dep_circle[1]-y_dep)) +
	  ((dep_circle[0]-pt_tan_dep[0])*(dep_circle[0]-pt_tan_dep[0])+
	   (dep_circle[1]-pt_tan_dep[1])*(dep_circle[1]-pt_tan_dep[1])) -
	  ((x_dep-pt_tan_dep[0])*(x_dep-pt_tan_dep[0])+
	   (y_dep-pt_tan_dep[1])*(y_dep-pt_tan_dep[1]))) /
	 (2*sqrt((dep_circle[0]-x_dep)*(dep_circle[0]-x_dep)+
	         (dep_circle[1]-y_dep)*(dep_circle[1]-y_dep)) *
	    sqrt((dep_circle[0]-pt_tan_dep[0])*(dep_circle[0]-pt_tan_dep[0])+
	         (dep_circle[1]-pt_tan_dep[1])*(dep_circle[1]-pt_tan_dep[1]))));

	angle_dest = acos(
	 (((dest_circle[0]-pt_tan_dest[0])*(dest_circle[0]-pt_tan_dest[0])+
	   (dest_circle[1]-pt_tan_dest[1])*(dest_circle[1]-pt_tan_dest[1])) +
	  ((dest_circle[0]-x_dest)*(dest_circle[0]-x_dest)+
	   (dest_circle[1]-y_dest)*(dest_circle[1]-y_dest)) -
	  ((pt_tan_dest[0]-x_dest)*(pt_tan_dest[0]-x_dest)+
	   (pt_tan_dest[1]-y_dest)*(pt_tan_dest[1]-y_dest))) /
	 (2*sqrt((dest_circle[0]-pt_tan_dest[0])*(dest_circle[0]-pt_tan_dest[0])+
	         (dest_circle[1]-pt_tan_dest[1])*(dest_circle[1]-pt_tan_dest[1])) *
	    sqrt((dest_circle[0]-x_dest)*(dest_circle[0]-x_dest)+
	         (dest_circle[1]-y_dest)*(dest_circle[1]-y_dest))));

	dep_len = r_dep * angle_dep;
	straight_len = sqrt(
	 (pt_tan_dest[0]-pt_tan_dep[0])*(pt_tan_dest[0]-pt_tan_dep[0]) +
	 (pt_tan_dest[1]-pt_tan_dep[1])*(pt_tan_dest[1]-pt_tan_dep[1]));
	dest_len = r_dest * angle_dest;
	tot_len = dep_len + straight_len + dest_len;

	dep_pts = (dep_len/tot_len) * n_pts;
	if (dep_pts == 0) {
		dep_pts++;
	}
	straight_pts = (straight_len/tot_len) * n_pts;
	if (straight_pts == 0) {
		straight_pts++;
	}
	dest_pts = (dest_len/tot_len) * n_pts;
	if (dest_pts == 0) {
		dest_pts++;
	}

	i = 1;
}

// Update distance and angle goals: called every 50ms
void update_sub_coordinates(void) {
	if (i <= dep_pts) {
		angle_goal += dep_to_left*angle_dep*L_MM/(U_MM*dep_pts);
		dist_goal += dep_len / dep_pts;
	} else if (i <= dep_pts + straight_pts) {
		dist_goal += straight_len / straight_pts;
	} else {
		angle_goal += dest_to_left*angle_dest *L_MM/(U_MM*dest_pts);
		dist_goal += dest_len / dest_pts;
	}

	angle_goal += last_angle_error;
	dist_goal += last_dist_error;

	i++;
}
