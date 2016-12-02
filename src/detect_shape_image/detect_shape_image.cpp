/* Detect the shape of the ballon in a picture:
 * - convert the picture to greyscale
 * - clean the picture through morphological opening and closing
 * - detect white shapes
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;

enum {
	STRUCT_ELEM_SIZE = 5
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

	/* Structuring element for morphological operations */
	struct_elem = getStructuringElement(
		MORPH_ELLIPSE, Size(STRUCT_ELEM_SIZE, STRUCT_ELEM_SIZE));

	/* morphological opening: remove small objects */
	erode(pic, pic, struct_elem);
	dilate(pic, pic, struct_elem);

	/* morphological closing: fill in small holes */
	dilate(pic, pic, struct_elem);
	erode(pic, pic, struct_elem);

	imshow("display", pic);
	waitKey(0);

	return 0;
}

