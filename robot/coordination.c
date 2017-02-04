#include "trigo.h"
#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"
#include "radiocomms.h"
#include "dance.h"

#ifdef DEBUG_ACH
#include "RTT/SEGGER_RTT.h"
#endif // DEBUG_ACH

// return 1 if x >= 0, -1 otherwise
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int dist_goal = 0;        // PID
volatile int angle_goal = 0;       // PID->tick diff: 0 is straigt, 246 is Pi/2
int last_angle_error = 0;          // computed in position.c
int last_dist_error = 0;           // computed in position.c

float orientation;                 // orientation of the robot in rad
#ifndef DEBUG_ACH
int x_pos, y_pos;                  // last measured position
#else
volatile int x_pos, y_pos;
#endif // DEBUG_ACH

int pt = 0;                        // current sub point

static int xdep, ydep, xdest, ydest;        // make the parameters global
static int rdep, rdest, npts;
static float oridest;
static int depnpts, straightnpts, destnpts; // number of points in each section
static int tandep[2], tandest[2];           // tangent points
static int isinnertan;                      // 1/-1 inner or outer tangent
static int depleft, destleft;               // 1/-1 going to the left or right
static float angledep, angledest;
static int totlen, deplen, straightlen, destlen;

#ifdef DEBUG_ACH
static int dbcdep[2], dbcdest[2];
int dbtandep[2], dbtandest[2];
#endif // DEBUG_ACH

// Called once to set the destination, return the number of points
int compute_traj(void)
{
#ifdef DEBUG_ACH
	printf("Compute trajectory\n");
#endif // DEBUG_ACH
	int j; // loop index
	static int cdep[2], cdest[2];           // center of the circles
	int tmp1[2], tmp2[2], tmp3[2], tmp4[2]; // possible circles
	int h[2] = {0};                         // homothetic center
	int dirtyhack = 1;                      // solves rdep < re_dest
	// intermediary values to reduce the number of operations
	float depcos, depsin, destcos, destsin;
	int tan[2];                       // tandest - tandep
	int cdep_h[2], cdest_h[2];        // h[] - cdep[]
	int cdep_h2[2], cdest_h2[2];      // (h[] - cdep[])^2
	int dep_cdep2[2], dest_cdest2[2]; // (cdep[] - Xdep)^2
	int tan_cdep2[2], tan_cdest2[2];  // (cdep[] - tandep[])^2
	int tan_dep2[2];                  // (Xdep - tandep[])^2
	int dest_tan2[2];                 // (tandest[] - Xdest)^2

	pt = 0;

	xdep = x_pos;
	ydep = y_pos;
	xdest = currentMove->x;
	ydest = currentMove->y;
	oridest = currentMove->angle * M_PI / 128;
	rdep = currentMove->startRadius;
	rdest = currentMove->endRadius;

	/* Because of the method of the homothetic center, both circles cannot
	 * be of the same radius, this hack solves the issue.
	 */
	if (rdep == rdest) {
		rdep++;
	}

	/* choose the right circles
	 * Find the closest circle, if an error is detected at the end,
	 * change circles.
	 */
	depcos = mcos(orientation);
	depsin = msin(orientation);
	tmp1[0] = xdep + rdep*depsin;
	tmp1[1] = ydep - rdep*depcos;
	tmp2[0] = xdep - rdep*depsin;
	tmp2[1] = ydep + rdep*depcos;
	if ((tmp1[0]-xdest)*(tmp1[0]-xdest) + (tmp1[1]-ydest)*(tmp1[1]-ydest) <=
	    (tmp2[0]-xdest)*(tmp2[0]-xdest) + (tmp2[1]-ydest)*(tmp2[1]-ydest))
	{
		depleft = -1;
		cdep[0] = tmp1[0];
		cdep[1] = tmp1[1];
	} else {
		depleft = 1;
		cdep[0] = tmp2[0];
		cdep[1] = tmp2[1];
	}

	destcos = mcos(oridest);
	destsin = msin(oridest);
	tmp3[0] = xdest + rdest*destsin;
	tmp3[1] = ydest - rdest*destcos;
	tmp4[0] = xdest - rdest*destsin;
	tmp4[1] = ydest + rdest*destcos;
	if ((tmp3[0]-xdep)*(tmp3[0]-xdep) + (tmp3[1]-ydep)*(tmp3[1]-ydep) <=
	    (tmp4[0]-xdep)*(tmp4[0]-xdep) + (tmp4[1]-ydep)*(tmp4[1]-ydep))
	{
		destleft = -1;
		cdest[0] = tmp3[0];
		cdest[1] = tmp3[1];
	} else {
		destleft = 1;
		cdest[0] = tmp4[0];
		cdest[1] = tmp4[1];
	}

	/* find the tangent points
	 * Hypothesis: the robot will never need to to go to the left if
	 * the goal is in the right-hand quadrant, nor behind it.
	 * At most two passes are needed.
	 */
	for (j = 0; j < 2; j++) {
		if (depleft * destleft == 1) {
			isinnertan = -1;
		} else {
			isinnertan = 1;
		}
	
		h[0] = (rdep*cdest[0] + isinnertan*rdest*cdep[0]) /
		       (rdep + isinnertan*rdest);
		h[1] = (rdep*cdest[1] + isinnertan*rdest*cdep[1]) /
		       (rdep + isinnertan*rdest);

		cdep_h[0] = h[0] - cdep[0];
		cdep_h[1] = h[1] - cdep[1];
		cdep_h2[0] = cdep_h[0] * cdep_h[0];
		cdep_h2[1] = cdep_h[1] * cdep_h[1];

		cdest_h[0] = h[0] - cdest[0];
		cdest_h[1] = h[1] - cdest[1];
		cdest_h2[0] = cdest_h[0] * cdest_h[0];
		cdest_h2[1] = cdest_h[1] * cdest_h[1];

		tandep[0] = cdep[0] +
		 (rdep*rdep*cdep_h[0] +
		  dirtyhack*depleft*rdep*cdep_h[1]*
		  sqrt(cdep_h2[0] + cdep_h2[1] - rdep*rdep)) /
		 (cdep_h2[0] + cdep_h2[1]);

		tandep[1] = cdep[1] +
		 (rdep*rdep*cdep_h[1] -
		  dirtyhack*depleft*rdep*cdep_h[0]*
		  sqrt(cdep_h2[0] + cdep_h2[1] - rdep*rdep)) /
		 (cdep_h2[0] + cdep_h2[1]);
	
		tandest[0] = cdest[0] +
		 (rdest*rdest*cdest_h[0] -
		  dirtyhack*isinnertan*destleft*rdest*cdest_h[1]*
		  sqrt(cdest_h2[0] + cdest_h2[1] - rdest*rdest)) /
		 (cdest_h2[0] + cdest_h2[1]);

		tandest[1] = cdest[1] +
		 (rdest*rdest*cdest_h[1] +
		  dirtyhack*isinnertan*destleft*rdest*cdest_h[0]*
		  sqrt(cdest_h2[0] + cdest_h2[1] - rdest*rdest)) /
		 (cdest_h2[0] + cdest_h2[1]);
	
		/* Correct the error if the wrong circle was chosen.
		 * Happens when the direction goes through the opposite circle.
		 */
		if (tan[0]*depcos + tan[1]*depsin > 0 &&
		    (tandep[0]-xdep)*depcos + (tandep[1]-ydep)*depsin < 0)
		{
			depleft *= -1;
			if (cdep[0] == tmp1[0] && cdep[1] == tmp1[1]) {
				cdep[0] = tmp2[0];
				cdep[1] = tmp2[1];
			} else {
				cdep[0] = tmp1[0];
				cdep[1] = tmp1[1];
			}
			continue;
		}
		if (tan[0]*destcos + tan[1]*destsin > 0 &&
		   (xdest-tandest[0])*destcos + (ydest-tandest[1])*destsin < 0)
		{
			destleft *= -1;
			if (cdest[0] == tmp3[0] && cdest[1] == tmp3[1]) {
				cdest[0] = tmp4[0];
				cdest[1] = tmp4[1];
			} else {
				cdest[0] = tmp3[0];
				cdest[1] = tmp3[1];
			}
			continue;
		}
	
		break;
	}

#ifdef DEBUG_ACH
	dbcdep[0] = cdep[0];
	dbcdep[1] = cdep[1];
	dbcdest[0] = cdest[0];
	dbcdest[1] = cdest[1];
#endif // DEBUG_ACH

	dep_cdep2[0] = cdep[0] - xdep;
	dep_cdep2[1] = cdep[1] - ydep;
	dep_cdep2[0] *= dep_cdep2[0];
	dep_cdep2[1] *= dep_cdep2[1];

	dest_cdest2[0] = cdest[0] - xdest;
	dest_cdest2[1] = cdest[1] - ydest;
	dest_cdest2[0] *= dest_cdest2[0];
	dest_cdest2[1] *= dest_cdest2[1];

	tan_cdep2[0] = cdep[0] - tandep[0];
	tan_cdep2[1] = cdep[1] - tandep[1];
	tan_cdep2[0] *= tan_cdep2[0];
	tan_cdep2[1] *= tan_cdep2[1];

	tan_cdest2[0] = cdest[0] - tandest[0];
	tan_cdest2[1] = cdest[1] - tandest[1];
	tan_cdest2[0] *= tan_cdest2[0];
	tan_cdest2[1] *= tan_cdest2[1];

	dest_tan2[0] = tandest[0] - xdest;
	dest_tan2[1] = tandest[1] - ydest;
	dest_tan2[0] *= dest_tan2[0];
	dest_tan2[1] *= dest_tan2[1];


	angledep = macos((dep_cdep2[0]+dep_cdep2[1] +
	 tan_cdep2[0]+tan_cdep2[1] - tan_dep2[0]+tan_dep2[1]) /
	 (2*sqrt(dep_cdep2[0]+dep_cdep2[1])*
	    sqrt(tan_cdep2[0]+tan_cdep2[1])));

	angledest = macos((tan_cdest2[0]+tan_cdest2[1] +
	 dest_cdest2[0]+dest_cdest2[1] - dest_tan2[0]+dest_tan2[1]) /
	 (2*sqrt(tan_cdest2[0] + tan_cdest2[1]) *
	    sqrt(dest_cdest2[0] + dest_cdest2[1])));

	deplen = rdep * angledep;
	straightlen = sqrt(tan[0]*tan[0] + tan[1]*tan[1]);
	destlen = rdest * angledest;
	totlen = deplen + straightlen + destlen;

	npts = (currentMove->date * 100 - getDateMs()) / 50;
	depnpts = deplen * npts / totlen;
	if (depnpts == 0) {
		depnpts++;
	}
	straightnpts = straightlen * npts / totlen;
	if (straightnpts == 0) {
		straightnpts++;
	}
	destnpts = destlen* npts / totlen;
	if (destnpts == 0) {
		destnpts++;
	}

	return npts;

#ifdef DEBUG_ACH
	dbtandep[0] = tandep[0];
	dbtandep[1] = tandep[1];
	dbtandest[0] = tandest[0];
	dbtandest[1] = tandest[1];
#endif // DEBUG_ACH
}

// Update distance and angle goals: called every 50ms
void update_goal(void) {
#ifdef DEBUG_ACH
	printf("Update goal\n");
#endif // DEBUG_ACH
	if (pt <= depnpts) {
#ifndef DEBUG_ACH
		angle_goal += depleft*angledep/(U_RAD*depnpts);
		dist_goal += deplen / depnpts;
#else
		x_pos += dbcdep[0] +
			(mcos(angledep/depnpts) + msin(angledep/depnpts))*
			depleft*(x_pos-dbcdep[0]);
		y_pos += dbcdep[1] +
			(mcos(angledep/depnpts) + msin(angledep/depnpts))*
			depleft*(y_pos-dbcdep[1]);
		orientation += depleft * (angledep/depnpts);
#endif // DEBUG_ACH
	} else if (pt <= depnpts + straightnpts) {
#ifndef DEBUG_ACH
		dist_goal += straightlen / straightnpts;
#else
		x_pos += straightlen * mcos(orientation);
		y_pos += straightlen * msin(orientation);
#endif // DEBUG_ACH
	} else {
#ifndef DEBUG_ACH
		angle_goal += destleft*angledest/(U_RAD*destnpts);
		dist_goal += destlen / destnpts;
#else
		x_pos += dbcdest[0] +
			(mcos(angledest/destnpts) + msin(angledest/destnpts))*
			destleft*(x_pos-dbcdest[0]);
		y_pos += dbcdest[1] +
			(mcos(angledest/destnpts) + msin(angledest/destnpts))*
			destleft*(y_pos-dbcdest[1]);
		orientation += destleft * (angledest/depnpts);
#endif // DEBUG_ACH
	}
#ifndef DEBUG_ACH
	angle_goal += last_angle_error;
	dist_goal += last_dist_error;
#endif // DEBUG_ACH

	pt++;
}
