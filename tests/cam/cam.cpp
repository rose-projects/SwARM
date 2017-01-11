/* Detect the shape of the ballon in a video */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

enum {
	PIX_MAX_VAL = 255, /* maximum value for an 8bits pixel */
	RAD_MIN = 150, /* minimum radius of detectable circles */
	RAD_MAX = 200, /* maximum radius of detectable circles */
	ONE_MS = 1, /* 1 millisecond */
	ESC = 27 /* escape key */
};
const double SCALE_FACTOR = 0.3; /* frame shrinking scale factor */
int contrast_coef = 200; /* final coef is this / 100.0 to create a double */
int bright_coef = 0; /* range: [0, 510] to get [-255, 255] */
int hough_p2 = 10; /* circle: accumulator threshold for detection */
int hough_p1 = 90; /* circle: high threshold for Canny edge detector */

int
main(int argc, char *argv[])
{
	VideoCapture vid;
	Mat fm;
	namedWindow("control", WINDOW_AUTOSIZE);
	createTrackbar("contrast", "control", &contrast_coef, 300, NULL, NULL);
	createTrackbar("brightness", "control", &bright_coef, 510, NULL, NULL);
	createTrackbar("hough_p1", "control",
		&hough_p1, PIX_MAX_VAL - 1, NULL, NULL);
	createTrackbar("hough_p2", "control",
		&hough_p2, PIX_MAX_VAL - 1, NULL, NULL);
	namedWindow("display", WINDOW_AUTOSIZE);

	if (argc != 2) {
		printf("Usage: ./track <video path>\n");
		printf("note: use <0> for the default camera.");
		return -1;
	}

	/* the argument 0 opens the default video stream (here the camera) */
	if (!strncmp(argv[1], "0", 2))
		vid = VideoCapture(0);
	else
		vid = VideoCapture(argv[1]);
	if (!vid.isOpened()) {
		printf("No video\n");
		return -1;
	}

	for (;;) {
		vector<Vec3f> circles;

		if (vid.read(fm) == false) {
			printf("No fm\n");
			return -1;
		}

		fm.convertTo(fm, -1, contrast_coef / 100.0, bright_coef - 255);
		medianBlur(fm, fm, 5);
		cvtColor(fm, fm, COLOR_BGR2GRAY);

		/* hough_p1 cannot be 0, and the trackbar has to start at 0 */
		HoughCircles(fm, circles, HOUGH_GRADIENT, 1, fm.rows/4,
			hough_p1 + 1, hough_p2, RAD_MIN, RAD_MAX);

		for (auto const& c : circles)
			circle(fm, Point(c[0], c[1]), 2, Scalar(0, 255, 0),
				3, LINE_AA);

		imshow("display", fm);

		if (waitKey(ONE_MS) == ESC)
			break;
	}

	return 0;
}
