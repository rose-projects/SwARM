/* Detect the shape of the ballon in a video:
 * - convert the frame to greyscale
 * - use thresholding to get a binary frame
 * - clean frame with bilateral filtering (change for gaussian if too slow)
 * - get the center of circles
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

enum {
	PIX_MAX_VAL = 255, /* maximum value for an 8bits pixel */
	SIGMA_COLOUR = 20,
	SIGMA_SPACE = 20,
	HOUGH_P1 = 100, /* hough circle method parameter 1 */
	HOUGH_P2 = 200, /* hough circle method parameter 2 */
	ESC = 27 /* escape key */
};
const double SCALE_FACTOR = 0.3; /* frame shrinking scale factor */
int thresh = 190; /* threshold value, changed by a trackbar */

int
main(int argc, char *argv[])
{
	VideoCapture vid;
	Mat fm, blur_fm;
	namedWindow("display", WINDOW_AUTOSIZE);
	namedWindow("control", WINDOW_AUTOSIZE);
	const char* thresh_tb = "Threshold value";
	createTrackbar(thresh_tb, "control", &thresh, PIX_MAX_VAL, NULL, NULL);
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
	
		if (!fm.data) {
			printf("No image data\n");
			return -1;
		}

		cvtColor(fm, fm, COLOR_BGR2GRAY);

		threshold(fm, fm, (double) thresh, PIX_MAX_VAL, THRESH_BINARY);
		bilateralFilter(fm, blur_fm, 5, SIGMA_COLOUR, SIGMA_SPACE,
			BORDER_DEFAULT);
		HoughCircles(blur_fm, circles, HOUGH_GRADIENT, 1, fm.rows/8,
			HOUGH_P1, HOUGH_P2);

		resize(blur_fm, blur_fm, Size(), SCALE_FACTOR, SCALE_FACTOR,
			INTER_AREA);
		imshow("display", blur_fm);
		if (waitKey(30) == ESC)
			break;
	}

	return 0;
}
