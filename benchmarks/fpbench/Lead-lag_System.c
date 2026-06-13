#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float y, float yd) {
	float eps = 0.01f;
	float Dc = -1280.0f;
	float Ac00 = 0.499f;
	float Ac01 = -0.05f;
	float Ac10 = 0.01f;
	float Ac11 = 1.0f;
	float Bc0 = 1.0f;
	float Bc1 = 0.0f;
	float Cc0 = 564.48f;
	float Cc1 = 0.0f;
	float yc = 0.0f;
	float u = 0.0f;
	float xc0 = 0.0f;
	float xc1 = 0.0f;
	float i = 0.0f;
	float e = 1.0f;
	int tmp = e > eps;
	while (tmp) {
		float v = y - yd;
		float tmp_1;
		if (v < -1.0f) {
			tmp_1 = -1.0f;
		} else if (1.0f < v) {
			tmp_1 = 1.0f;
		} else {
			tmp_1 = v;
		}
		yc = tmp_1;
		u = (Cc0 * xc0) + ((Cc1 * xc1) + (Dc * yc));
		xc0 = (Ac00 * xc0) + ((Ac01 * xc1) + (Bc0 * yc));
		xc1 = (Ac10 * xc0) + ((Ac11 * xc1) + (Bc1 * yc));
		i = i + 1.0f;
		e = fabsf((yc - xc1));
		tmp = e > eps;
	}
	return xc1;
}

