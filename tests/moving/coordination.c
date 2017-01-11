#include "coordination.h"
#include "wheel_constants.h"
#include "asser.h"
#include "moving.h"
#include "position.h"
#include <math.h>
#include "hal.h"
#include "ch.h"
// To get the sign of a variable, returns 1 if positive or null, -1 if negative
#define SIGN(x) ((fabs(x)==x) ? 1 : -1)

volatile int xb = 10000;
volatile int yb = 20000;
volatile int distance = 0;
volatile int angle = 0;
volatile int dist_goal = 0;
volatile int angle_goal = 0;
volatile double orientation = 0;
volatile int x_pos = 0;
volatile int y_pos = 0;

static double xb_p;
static double yb_p;
static double radius;
static double alpha;
static int i;

void update_main_coordinates(){
    double theta = M_PI/2 - orientation;
    i = 1;
    xb_p = (xb - x_pos)*cos(theta) - (yb-y_pos)*sin(theta);
    yb_p = (xb - x_pos)*sin(theta) + (yb-y_pos)*cos(theta);

    radius = (fabs(xb_p) + yb_p*yb_p/fabs(xb_p))/2;
    alpha = 2*asin(sqrt((xb_p*xb_p)+(yb_p*yb_p))/(2*radius));
}

void update_sub_coordinates(){
    forward = SIGN(yb_p);
    dist_goal = fabs(alpha)*radius*i/N_POINTS;
    to_the_left = SIGN(xb_p);
    angle_goal = L_MM*dist_goal/(radius*U_MM)*to_the_left;
    i++;
}
