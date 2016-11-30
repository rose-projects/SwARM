/* First openCV program to discover the lib */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;

int
main(int argc, char *argv[])
{
	Mat pic;
	namedWindow("display", WINDOW_AUTOSIZE);

	if (argc != 2) {
		printf("Usage: ./first <image path>\n");
		return -1;
	}

	pic = imread(argv[1], 1);

	if (!pic.data) {
		printf("No image data\n");
		return -1;
	}

	imshow("display", pic);
	waitKey(0);

	return 0;
}
