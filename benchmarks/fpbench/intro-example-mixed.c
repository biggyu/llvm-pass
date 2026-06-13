#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float t) {
	float tmp_2 = t + 1.0f;
	double tmp_1 = ((double) t) / ((double) tmp_2);
	return (float) tmp_1;
}

