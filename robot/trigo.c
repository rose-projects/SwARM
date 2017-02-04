#include <stdint.h>

#define M_PI 3.1415926536f

const int16_t sinTab[65] = {0, 804, 1607, 2410, 3211, 4011, 4807, 5601, 6392, 7179, 7961, 8739, 9511, 10278, 11038, 11792, 12539, 13278, 14009, 14732, 15446, 16150, 16845, 17530, 18204, 18867, 19519, 20159, 20787, 21402, 22004, 22594, 23169, 23731, 24278, 24811, 25329, 25831, 26318, 26789, 27244, 27683, 28105, 28510, 28897, 29268, 29621, 29955, 30272, 30571, 30851, 31113, 31356, 31580, 31785, 31970, 32137, 32284, 32412, 32520, 32609, 32678, 32727, 32757, 32767};

static int16_t _sin(uint8_t x) {
	if(x <= 64)
		return sinTab[x];
	else if(x <= 128)
		return _sin(128 - x);
	else
		return -_sin(x - 128);
}

float msin(float x) {
	x = x*128.0/M_PI + 0.5;
	uint8_t val = x;
	return _sin(val)/32767.0;
}

float mcos(float x) {
	x = x*128.0/M_PI + 0.5;
	uint8_t val = x;
	val += 64;
	return _sin(val)/32767.0;
}

float macos(float x) {
	int val = x*32767, found = 0, start = 0, end = 128, middle;

	while(end - start > 1){
		middle = (start + end)/2;

		found = _sin(middle + 64);
		if(found == val)
			break;
		else if(found > val)
			start = middle;
		else
			end = middle;
	}

	return middle*M_PI/128;
}
