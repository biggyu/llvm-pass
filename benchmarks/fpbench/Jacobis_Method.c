#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float a11, float a22, float a33, float a44, float b1, float b2, float b3, float b4) {
	float eps = 1e-17f;
	float x_n1 = 0.0f;
	float x_n2 = 0.0f;
	float x_n3 = 0.0f;
	float x_n4 = 0.0f;
	float i = 0.0f;
	float e = 1.0f;
	float x1 = 0.0f;
	float x2 = 0.0f;
	float x3 = 0.0f;
	float x4 = 0.0f;
	int tmp = e > eps;
	while (tmp) {
		x_n1 = (((b1 / a11) - ((0.1f / a11) * x2)) - ((0.2f / a11) * x3)) + ((0.3f / a11) * x4);
		x_n2 = (((b2 / a22) - ((0.3f / a22) * x1)) + ((0.1f / a22) * x3)) - ((0.2f / a22) * x4);
		x_n3 = (((b3 / a33) - ((0.2f / a33) * x1)) + ((0.3f / a33) * x2)) - ((0.1f / a33) * x4);
		x_n4 = (((b4 / a44) + ((0.1f / a44) * x1)) - ((0.2f / a44) * x2)) - ((0.3f / a44) * x3);
		i = i + 1.0f;
		e = fabsf((x_n4 - x4));
		x1 = x_n1;
		x2 = x_n2;
		x3 = x_n3;
		x4 = x_n4;
		tmp = e > eps;
	}
	return x2;
}

