#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "coordination.h"
#include "wheel_constants.h"

extern int x_pos;
extern int y_pos;
extern double orientation;
extern int angle_goal;
extern int dist_goal;

int
main(void)
{
	int i = 0;
	int n_pts = 70;
	int x, y;

	x_pos = 10;
	y_pos = 10;

	x = x_pos;
	y = y_pos;
	orientation = M_PI_2;

	update_main_coordinates(200, 200, 120, 50, 20, n_pts);
	for (i = 0; i < n_pts; i++) {
		update_sub_coordinates();
		
		x += cos(orientation) * 2 * sin(angle_goal / 2) * dist_goal / angle_goal;
		y += sin(orientation) * 2 * sin(angle_goal / 2) * dist_goal / angle_goal;
		printf("%d\t%d\t%lf\n", x, y, orientation);
	}
	return 0;
}
