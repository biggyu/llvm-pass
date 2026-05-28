#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float h, float y_n_42_, float c) {
	float sixieme = 1.0f / 6.0f;
	float eps = 0.005f;
	float k = 1.2f;
	float y_n = y_n_42_;
	float i = 0.0f;
	float e = 1.0f;
	int tmp = e > eps;
	while (tmp) {
		float v = c - y_n;
		float k1 = (k * v) * v;
		float v_1 = c - (y_n + ((0.5f * h) * k1));
		float k2 = (k * v_1) * v_1;
		float v_2 = c - (y_n + ((0.5f * h) * k2));
		float k3 = (k * v_2) * v_2;
		float v_3 = c - (y_n + (h * k3));
		float k4 = (k * v_3) * v_3;
		float y_n_4 = y_n + ((sixieme * h) * (((k1 + (2.0f * k2)) + (2.0f * k3)) + k4));
		float i_5 = i + 1.0f;
		float e_6 = e - eps;
		y_n = y_n_4;
		i = i_5;
		e = e_6;
		tmp = e > eps;
	}
	return fabsf(e);
}

