#include <stdint.h>
#include <math.h>

const int16_t sinTab[65] = {0, 804, 1607, 2410, 3211, 4011, 4807, 5601, 6392,
        7179, 7961, 8739, 9511, 10278, 11038, 11792, 12539, 13278, 14009,
        14732, 15446, 16150, 16845, 17530, 18204, 18867, 19519, 20159, 20787,
        21402, 22004, 22594, 23169, 23731, 24278, 24811, 25329, 25831, 26318,
        26789, 27244, 27683, 28105, 28510, 28897, 29268, 29621, 29955, 30272,
        30571, 30851, 31113, 31356, 31580, 31785, 31970, 32137, 32284, 32412,
        32520, 32609, 32678, 32727, 32757, 32767};

static int16_t _sin(uint8_t x) {
	if(x <= 64)
		return sinTab[x];
	else if(x <= 128)
		return _sin(128 - x);
	else
		return -_sin(x - 128);
}

float msin(float x) {
	x = x*128.0/M_PI;
	while(x < 0) x += 256;
	while(x >= 256) x -= 256;

	uint8_t val = x;
	return (_sin(val) + (x - val)*(_sin(val+1)-_sin(val)))/32767.0;
}

float mcos(float x) {
	x = x*128.0/M_PI + 64;
	while(x < 0) x += 256;
	while(x >= 256) x -= 256;

	uint8_t val = x;
	return (_sin(val) + (x - val)*(_sin(val+1)-_sin(val)))/32767.0;
}

// this constant defines the required precision of the macos function
// (i.e. when the dichotomy can stop if it doesn't find the exact value)
#define MACOS_PRECISION 0.001

float macos(float x) {
	float found = 0, start = 0, end = 2*M_PI, middle = 0;

	while(start <= end){
		middle = (start + end)/2;

		found = mcos(middle);
		if(found == x)
			break;
		else if(found > x)
			start = middle + MACOS_PRECISION;
		else
			end = middle - MACOS_PRECISION;
	}

	return middle;
}

// this constant defines the required precision of the masin function
// (i.e. when the dichotomy can stop if it doesn't find the exact value)
#define MASIN_PRECISION 0.001

float masin(float x) {
	float found = 0, start = -M_PI/2, end = M_PI/2, middle = 0;

	while(start <= end){
		middle = (start + end)/2;

		found = msin(middle);
		if(found == x)
			break;
		else if(found < x)
			start = middle + MASIN_PRECISION;
		else
			end = middle - MASIN_PRECISION;
	}

	return middle;
}

float matan(float x) {
	const float b = 0.596227;

	if(x < 0)
		return -matan(-x);

	return (M_PI/2)*(b*x + x*x)/(1 + 2*b*x + x*x);
}
