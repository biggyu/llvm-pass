#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float x0) {
	float eps = 0.0005f;
	float x_n = 0.0f;
	float e = 1.0f;
	float x = 0.0f;
	float i = 0.0f;
	int tmp = (e > eps) && (i < 100000.0f);
	while (tmp) {
		float f = ((((((x * x) * ((x * x) * x)) - ((10.0f * x) * ((x * x) * x))) + ((40.0f * x) * (x * x))) - ((80.0f * x) * x)) + (80.0f * x)) - 32.0f;
		float ff = (((((5.0f * x) * ((x * x) * x)) - ((40.0f * x) * (x * x))) + ((120.0f * x) * x)) - (160.0f * x)) + 80.0f;
		x_n = x - (f / ff);
		e = fabsf((x - x_n));
		x = x_n;
		i = i + 1.0f;
		tmp = (e > eps) && (i < 100000.0f);
	}
	return x;
}

