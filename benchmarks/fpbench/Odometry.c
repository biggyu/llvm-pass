#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float sr_42_, float sl_42_) {
	float inv_l = 0.1f;
	float c = 12.34f;
	float delta_dl = 0.0f;
	float delta_dr = 0.0f;
	float delta_d = 0.0f;
	float delta_theta = 0.0f;
	float arg = 0.0f;
	float cosi = 0.0f;
	float x = 0.0f;
	float sini = 0.0f;
	float y = 0.0f;
	float theta = -0.985f;
	float t = 0.0f;
	float tmp = sl_42_;
	float sl = sl_42_;
	float sr = sr_42_;
	float j = 0.0f;
	int tmp_1 = t < 1000.0f;
	while (tmp_1) {
		delta_dl = c * sl;
		delta_dr = c * sr;
		delta_d = (delta_dl + delta_dr) * 0.5f;
		delta_theta = (delta_dr - delta_dl) * inv_l;
		arg = theta + (delta_theta * 0.5f);
		cosi = (1.0f - ((arg * arg) * 0.5f)) + ((((arg * arg) * arg) * arg) * 0.0416666666f);
		x = x + (delta_d * cosi);
		sini = (arg - (((arg * arg) * arg) * 0.1666666666f)) + (((((arg * arg) * arg) * arg) * arg) * 0.008333333f);
		y = y + (delta_d * sini);
		theta = theta + delta_theta;
		t = t + 1.0f;
		tmp = sl;
		float tmp_2;
		if (j == 50.0f) {
			tmp_2 = sr;
		} else {
			tmp_2 = sl;
		}
		sl = tmp_2;
		float tmp_3;
		if (j == 50.0f) {
			tmp_3 = tmp;
		} else {
			tmp_3 = sr;
		}
		sr = tmp_3;
		float tmp_4;
		if (j == 50.0f) {
			tmp_4 = 0.0f;
		} else {
			tmp_4 = j + 1.0f;
		}
		j = tmp_4;
		tmp_1 = t < 1000.0f;
	}
	return x;
}

