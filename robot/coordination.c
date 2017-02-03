#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"

#include <math.h>

// return 1 if x >= 0, -1 otherwise
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int dist_goal = 0;        // PID
volatile int angle_goal = 0;       // PID->tick diff: 0 is straigt, 246 is Pi/2
volatile int last_angle_error = 0; // computed in position.c
volatile int last_dist_error = 0;  // computed in position.c

double orientation = 0;   // orientation of the robot in rad
int x_pos = 0, y_pos = 0; // last measured position

static int i;                               // i-th point of the trajectory
static int x_dep, y_dep, x_dest, y_dest;    // make the parameters global
static int r_dep, r_dest, n_pts;
static float ori_dest;
static int dep_pts, straight_pts, dest_pts; // number of points in each section
static int c_dep[2], c_dest[2];             // center of the circles
static int pt_tan_dep[2], pt_tan_dest[2];   // tangent points
static int is_inner_tan;                    // 1/-1 inner or outer tangent
static int dep_to_left, dest_to_left;       // 1/-1 going to the left or right
static float angle_dep, angle_dest;
static int tot_len, dep_len, straight_len, dest_len;

#ifdef DEBUG
int db_pt_tan_dep[2], db_pt_tan_dest[2]; // debug
#endif // DEBUG

// Called once to set the destination
void update_main_coordinates(int x_dest_, int y_dest_, float ori_dest_,
                             int r_dep_, int r_dest_, int n_pts_)
{
	int j; // loop index
	int tmp1[2], tmp2[2], tmp3[2], tmp4[2]; // possible circles
	int h[2] = {0};                         // homothetic center
	int dirty_hack = 1;                     // solves r_dep < re_dest
	// intermediary values to reduce the number of operations
	float dep_cos, dep_sin, dest_cos, dest_sin;
	int tan[2];                           // pt_tan_dest - pt_tan_dep
	int c_dep_h[2], c_dest_h[2];          // h[] - c_dep[]
	int c_dep_h_2[2], c_dest_h_2[2];      // (h[] - c_dep[])^2
	int dep_c_dep_2[2], dest_c_dest_2[2]; // (c_dep[] - X_dep)^2
	int tan_c_dep_2[2], tan_c_dest_2[2];  // (c_dep[] - pt_tan_dep[])^2
	int tan_dep_2[2];                     // (X_dep - pt_tan_dep[])^2
	int dest_tan_2[2];                    // (pt_tan_dest[] - X_dest)^2

	/* Because of the method of the homothetic center, both circles cannot
	 * be of the same radius, this hack solves the issue.
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
	 * Find the closest circle, if an error is detected at the end,
	 * change circles.
	 */
	dep_cos = sin(M_PI_2 - orientation);
	dep_sin = sin(orientation);
	tmp1[0] = x_dep + r_dep*dep_sin;
	tmp1[1] = y_dep - r_dep*dep_cos;
	tmp2[0] = x_dep - r_dep*dep_sin;
	tmp2[1] = y_dep + r_dep*dep_cos;
	if ((tmp1[0]-x_dest)*(tmp1[0]-x_dest)+
	    (tmp1[1]-y_dest)*(tmp1[1]-y_dest) <=
	    (tmp2[0]-x_dest)*(tmp2[0]-x_dest)+
	    (tmp2[1]-y_dest)*(tmp2[1]-y_dest))
	{
		dep_to_left = -1;
		c_dep[0] = tmp1[0];
		c_dep[1] = tmp1[1];
	} else {
		dep_to_left = 1;
		c_dep[0] = tmp2[0];
		c_dep[1] = tmp2[1];
	}

	dest_cos = sin(M_PI_2 - ori_dest);
	dest_sin = sin(ori_dest);
	tmp3[0] = x_dest + r_dest*dest_sin;
	tmp3[1] = y_dest - r_dest*dest_cos;
	tmp4[0] = x_dest - r_dest*dest_sin;
	tmp4[1] = y_dest + r_dest*dest_cos;
	if ((tmp3[0]-x_dep)*(tmp3[0]-x_dep)+
	    (tmp3[1]-y_dep)*(tmp3[1]-y_dep) <=
	    (tmp4[0]-x_dep)*(tmp4[0]-x_dep)+
	    (tmp4[1]-y_dep)*(tmp4[1]-y_dep))
	{
		dest_to_left = -1;
		c_dest[0] = tmp3[0];
		c_dest[1] = tmp3[1];
	} else {
		dest_to_left = 1;
		c_dest[0] = tmp4[0];
		c_dest[1] = tmp4[1];
	}

	/* find the tangent points
	 * Hypothesis: the robot will never need to to go to the left if
	 * the goal is in the right-hand quadrant, nor behind it.
	 * At most two passes are needed.
	 */
	for (j = 0; j < 2; j++) {
		if (dep_to_left * dest_to_left == 1) {
			is_inner_tan = -1;
		} else {
			is_inner_tan = 1;
		}
	
		h[0] = (r_dep*c_dest[0] + is_inner_tan*r_dest*c_dep[0]) /
		       (r_dep + is_inner_tan*r_dest);
		h[1] = (r_dep*c_dest[1] + is_inner_tan*r_dest*c_dep[1]) /
		       (r_dep + is_inner_tan*r_dest);

		c_dep_h[0] = h[0] - c_dep[0];
		c_dep_h[1] = h[1] - c_dep[1];
		c_dep_h_2[0] = c_dep_h[0] * c_dep_h[0];
		c_dep_h_2[1] = c_dep_h[1] * c_dep_h[1];

		c_dest_h[0] = h[0] - c_dest[0];
		c_dest_h[1] = h[1] - c_dest[1];
		c_dest_h_2[0] = c_dest_h[0] * c_dest_h[0];
		c_dest_h_2[1] = c_dest_h[1] * c_dest_h[1];

		pt_tan_dep[0] = c_dep[0] +
		 (r_dep*r_dep*c_dep_h[0] +
		  dirty_hack*dep_to_left*r_dep*c_dep_h[1]*
		  sqrt(c_dep_h_2[0] + c_dep_h_2[1] - r_dep*r_dep)) /
		 (c_dep_h_2[0] + c_dep_h_2[1]);

		pt_tan_dep[1] = c_dep[1] +
		 (r_dep*r_dep*c_dep_h[1] -
		  dirty_hack*dep_to_left*r_dep*c_dep_h[0]*
		  sqrt(c_dep_h_2[0] + c_dep_h_2[1] - r_dep*r_dep)) /
		 (c_dep_h_2[0] + c_dep_h_2[1]);
	
		pt_tan_dest[0] = c_dest[0] +
		 (r_dest*r_dest*c_dest_h[0] -
		  dirty_hack*is_inner_tan*dest_to_left*r_dest*c_dest_h[1]*
		  sqrt(c_dest_h_2[0] + c_dest_h_2[1] - r_dest*r_dest)) /
		 (c_dest_h_2[0] + c_dest_h_2[1]);

		pt_tan_dest[1] = c_dest[1] +
		 (r_dest*r_dest*c_dest_h[1] +
		  dirty_hack*is_inner_tan*dest_to_left*r_dest*c_dest_h[0]*
		  sqrt(c_dest_h_2[0] + c_dest_h_2[1] - r_dest*r_dest)) /
		 (c_dest_h_2[0] + c_dest_h_2[1]);
	
		/* Correct the error if the wrong circle was chosen.
		 * Happens when the direction goes through the opposite circle.
		 */
		if (tan[0]*dep_cos + tan[1]*dep_sin > 0 &&
		    (pt_tan_dep[0]-x_dep)*dep_cos +
		    (pt_tan_dep[1]-y_dep)*dep_sin < 0)
		{
			dep_to_left *= -1;
			if (c_dep[0] == tmp1[0] && c_dep[1] == tmp1[1]) {
				c_dep[0] = tmp2[0];
				c_dep[1] = tmp2[1];
			} else {
				c_dep[0] = tmp1[0];
				c_dep[1] = tmp1[1];
			}
			continue;
		}
		if (tan[0]*dest_cos + tan[1]*dest_sin > 0 &&
		   (x_dest-pt_tan_dest[0])*dest_cos +
		   (y_dest-pt_tan_dest[1])*dest_sin < 0)
		{
			dest_to_left *= -1;
			if (c_dest[0] == tmp3[0] && c_dest[1] == tmp3[1]) {
				c_dest[0] = tmp4[0];
				c_dest[1] = tmp4[1];
			} else {
				c_dest[0] = tmp3[0];
				c_dest[1] = tmp3[1];
			}
			continue;
		}
	
		break;
	}

	dep_c_dep_2[0] = c_dep[0] - x_dep;
	dep_c_dep_2[1] = c_dep[1] - y_dep;
	dep_c_dep_2[0] *= dep_c_dep_2[0];
	dep_c_dep_2[1] *= dep_c_dep_2[1];

	dest_c_dest_2[0] = c_dest[0] - x_dest;
	dest_c_dest_2[1] = c_dest[1] - y_dest;
	dest_c_dest_2[0] *= dest_c_dest_2[0];
	dest_c_dest_2[1] *= dest_c_dest_2[1];

	tan_c_dep_2[0] = c_dep[0] - pt_tan_dep[0];
	tan_c_dep_2[1] = c_dep[1] - pt_tan_dep[1];
	tan_c_dep_2[0] *= tan_c_dep_2[0];
	tan_c_dep_2[1] *= tan_c_dep_2[1];

	tan_c_dest_2[0] = c_dest[0] - pt_tan_dest[0];
	tan_c_dest_2[1] = c_dest[1] - pt_tan_dest[1];
	tan_c_dest_2[0] *= tan_c_dest_2[0];
	tan_c_dest_2[1] *= tan_c_dest_2[1];

	dest_tan_2[0] = pt_tan_dest[0] - x_dest;
	dest_tan_2[1] = pt_tan_dest[1] - y_dest;
	dest_tan_2[0] *= dest_tan_2[0];
	dest_tan_2[1] *= dest_tan_2[1];


	angle_dep = acos((dep_c_dep_2[0]+dep_c_dep_2[1] +
	 tan_c_dep_2[0]+tan_c_dep_2[1] - tan_dep_2[0]+tan_dep_2[1]) /
	 (2*sqrt(dep_c_dep_2[0]+dep_c_dep_2[1])*
	    sqrt(tan_c_dep_2[0]+tan_c_dep_2[1])));

	angle_dest = acos((tan_c_dest_2[0]+tan_c_dest_2[1] +
	 dest_c_dest_2[0]+dest_c_dest_2[1] - dest_tan_2[0]+dest_tan_2[1]) /
	 (2*sqrt(tan_c_dest_2[0] + tan_c_dest_2[1]) *
	    sqrt(dest_c_dest_2[0] + dest_c_dest_2[1])));

	dep_len = r_dep * angle_dep;
	straight_len = sqrt(tan[0]*tan[0] + tan[1]*tan[1]);
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

#ifdef DEBUG
	db_pt_tan_dep[0] = pt_tan_dep[0];
	db_pt_tan_dep[1] = pt_tan_dep[1];
	db_pt_tan_dest[0] = pt_tan_dest[0];
	db_pt_tan_dest[1] = pt_tan_dest[1];
#endif //DEBUG
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
