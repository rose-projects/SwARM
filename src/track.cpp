/* Detect the shape of the ballon in a video */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

enum {
	PIX_MAX_VAL = 255, /* maximum value for an 8bits pixel */
	HOUGH_P1 = 80, /* circle: high threshold for Canny edge detector */
	HOUGH_P2 = 20, /* circle: accumulator threshold for detection */
	RAD_MIN = 200, /* minimum radius of detectable circles */
	RAD_MAX = 500, /* maximum radius of detectable circles */
	ONE_MS = 1, /* 1 millisecond */
	ESC = 27 /* escape key */
};
const double SCALE_FACTOR = 0.3; /* frame shrinking scale factor */

int
main(int argc, char *argv[])
{
	size_t i = 0;
	VideoCapture vid;
	Mat fm;
	namedWindow("display", WINDOW_AUTOSIZE);
	vector<Vec3f> circles;

	if (argc != 2) {
		printf("Usage: ./track <video path>\n");
		return -1;
	}

	vid = VideoCapture(argv[1]);
	if (!vid.isOpened()) {
		printf("No video\n");
		return -1;
	}

	for (;;) {
		if (vid.read(fm) == false) {
			printf("No fm\n");
			return -1;
		}

		cvtColor(fm, fm, COLOR_BGR2GRAY);

		HoughCircles(fm, circles, HOUGH_GRADIENT, 1, fm.rows/8,
			HOUGH_P1, HOUGH_P2, RAD_MIN, RAD_MAX);

		for (i = 0; i < circles.size(); i++) {
			Vec3i c = circles[i];
			circle(fm, Point(c[0], c[1]), 2, Scalar(0, 255, 0),
				3, LINE_AA);
		}

		resize(fm, fm, Size(), SCALE_FACTOR, SCALE_FACTOR,
			INTER_AREA);
		imshow("display", fm);

		if (waitKey(ONE_MS) == ESC)
			break;
	}

	return 0;
}
