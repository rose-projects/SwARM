/* Detect the shape of the ballon in a video:
 * - convert the frame to greyscale
 * - use thresholding to get a binary frame
 * - clean frame with morphological operations
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;

enum {
	MAX_VAL = 255,		/* maximum value for an 8bits pixel */
	THRESH = 190,		/* threshold value */
	STRUCT_ELEM_S = 3	/* structuring element size */
};
const double SCALE_FACTOR = 0.3;	/* frame shrinking scale factor */

Mat morph_clean(Mat m);

int
main(int argc, char *argv[])
{
	VideoCapture vid;
	Mat fm;
	namedWindow("display", WINDOW_AUTOSIZE);

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

		threshold(fm, fm, THRESH, MAX_VAL, THRESH_BINARY | THRESH_OTSU);
		fm = morph_clean(fm);

		resize(fm, fm, Size(), SCALE_FACTOR, SCALE_FACTOR, INTER_AREA);
		imshow("display", fm);
		if (waitKey(30) == 27)
			break;
	}

	return 0;
}

Mat
morph_clean(Mat m)
{
	Mat struct_elem;

	struct_elem = getStructuringElement(MORPH_ELLIPSE,
		Size(STRUCT_ELEM_S, STRUCT_ELEM_S));

	/* morphological opening: remove small spots */
	erode(m, m, struct_elem);
	dilate(m, m, struct_elem);

	/* morphological closing: fill small holes */
	dilate(m, m, struct_elem);
	erode(m, m, struct_elem);

	return m;
}
