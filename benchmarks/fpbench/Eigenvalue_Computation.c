#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float a11, float a12, float a13, float a14, float a21, float a22, float a23, float a24, float a31, float a32, float a33, float a34, float a41, float a42, float a43, float a44, float v1, float v2, float v3, float v4) {
	float eps = 0.0005f;
	float vx = 0.0f;
	float vy = 0.0f;
	float vz = 0.0f;
	float vw = 0.0f;
	float i = 0.0f;
	float v1_1 = v1;
	float v2_2 = v2;
	float v3_3 = v3;
	float v4_4 = v4;
	float e = 1.0f;
	int tmp = e > eps;
	while (tmp) {
		vx = ((a11 * v1_1) + (a12 * v2_2)) + ((a13 * v3_3) + (a14 * v4_4));
		vy = ((a21 * v1_1) + (a22 * v2_2)) + ((a23 * v3_3) + (a24 * v4_4));
		vz = ((a31 * v1_1) + (a32 * v2_2)) + ((a33 * v3_3) + (a34 * v4_4));
		vw = ((a41 * v1_1) + (a42 * v2_2)) + ((a43 * v3_3) + (a44 * v4_4));
		i = i + 1.0f;
		v1_1 = vx / vw;
		v2_2 = vy / vw;
		v3_3 = vz / vw;
		v4_4 = 1.0f;
		e = fabsf((1.0f - v1_1));
		tmp = e > eps;
	}
	return v1_1;
}

