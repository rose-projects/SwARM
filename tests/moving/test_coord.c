#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "coordination.h"
#include "wheel_constants.h"

extern int x_pos, y_pos;
extern double orientation;
extern int angle_goal, dist_goal;
extern int db_pt_tan_dep[2], db_pt_tan_dest[2];

int
main(int argc, char *argv[])
{
	int x, y;

	if (argc != 2) {
		printf("Usage: ./a.out <number of points>\n");
		return -1;
	}

	x_pos = 100;
	y_pos = 100;

	x = x_pos;
	y = y_pos;
	orientation = M_PI_2;


	update_main_coordinates(200, 200, 120, 50, 20, atoi(argv[1]));
	printf("%d %d\n", db_pt_tan_dep[0], db_pt_tan_dep[1]);
	printf("%d %d\n", db_pt_tan_dest[0], db_pt_tan_dest[1]);
	return 0;
}
