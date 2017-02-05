#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "coordination.h"
#include "wheel_constants.h"

extern int x_pos, y_pos;
extern float orientation;
extern int angle_goal, dist_goal;
extern int dbtandep[2], dbtandest[2];

int
main(int argc, char *argv[])
{
	(void) argv;

	if (argc != 1) {
		printf("Usage: ./a.out\n");
		return -1;
	}

	x_pos = 100;
	y_pos = 100;
	orientation = M_PI_2;

	compute_traj();
	printf("%d %d\n", dbtandep[0], dbtandep[1]);
	printf("%d %d\n", dbtandest[0], dbtandest[1]);
	return 0;
}
