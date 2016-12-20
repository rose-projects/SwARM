#include "asser.h"
#include "coding_wheels.h"
#include "wheel_constants.h"

#include "ch.h"

// ASSER frequency in Hz
#define ASSER_FREQ          200
// ASSER THREADS sleep time in ms
#define ASSER_THD_SLEEP     (1000/ASSER_FREQ)

static THD_WORKING_AREA(working_area_angle_l, 128);
//static THD_WORKING_AREA(working_area_distance_l, 128);
static THD_WORKING_AREA(working_area_angle_r, 128);
//static THD_WORKING_AREA(working_area_distance_r, 128);

// Robot's speed in m/s
volatile int speed_l = 0;
volatile int speed_r = 0;
volatile int speed = 0;

// Angle asserv thread for LEFT WHEEL
static THD_FUNCTION(angle_thread_l, arg) {
    (void) arg;
    int last_tick = 0;

    // 100 Hz calculation
    while(true){
        // Update left wheel speed
        speed_l = (tick_l - last_tick)*U_MM*ASSER_FREQ;
        last_tick = tick_l;
        
        // Go to sleep
        chThdSleepMilliseconds(ASSER_THD_SLEEP);
    }
}

/*
// Distance asserv thread for LEFT WHEEL
static THD_FUNCTION(distance_thread_l, arg) {
}
*/

// Angle asserv thread for RIGHT WHEEL
static THD_FUNCTION(angle_thread_r, arg) {
    (void) arg;
    int last_tick = 0;

    // 100 Hz calculation
    while(true){
        // Update left wheel speed
        speed_r = (tick_r - last_tick)*U_MM*ASSER_FREQ;
        last_tick = tick_r;
        
        // Go to sleep
        chThdSleepMilliseconds(ASSER_THD_SLEEP);
    }
}

/*
// Distance asserv thread for RIGHT WHEEL
static THD_FUNCTION(distance_thread_r, arg) {
}
*/

void start_asservs(){
    // Starting the monitoring threads
    (void)chThdCreateStatic(working_area_angle_l, sizeof(working_area_angle_l),
            NORMALPRIO, angle_thread_l, NULL);
    (void)chThdCreateStatic(working_area_angle_r, sizeof(working_area_angle_r),
            NORMALPRIO, angle_thread_r, NULL);
}
