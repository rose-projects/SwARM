/* Detect the shape of the ballon in a picture:
 * - convert the picture to greyscale
 * - use thresholding to get a binary picture
 * - clean picture with morphological operations
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;

enum {
	MAX_VAL = 255,		/* maximum value for an 8bits pixel */
	THRESH = 190,		/* threshold value */
	STRUCT_ELEM_S = 3	/* structuring element size */
};

int
main(int argc, char *argv[])
{
	Mat pic, struct_elem;
	namedWindow("display", WINDOW_AUTOSIZE);

	if (argc != 2) {
		printf("Usage: ./detect_shape_image <image path>\n");
		return -1;
	}

	pic = imread(argv[1], 1);
	if (!pic.data) {
		printf("No image data\n");
		return -1;
	}

	cvtColor(pic, pic, COLOR_BGR2GRAY);
	threshold(pic, pic, THRESH, MAX_VAL, THRESH_BINARY);

	/* morphological structuring element: circle */
	struct_elem = getStructuringElement(MORPH_ELLIPSE,
		Size(STRUCT_ELEM_S, STRUCT_ELEM_S));

	/* morphological opening: remove small spots */
	erode(pic, pic, struct_elem);
	dilate(pic, pic, struct_elem);

	/* morphological closing: fill small holes */
	dilate(pic, pic, struct_elem);
	erode(pic, pic, struct_elem);

	imshow("display", pic);
	waitKey(0);

	return 0;
}

