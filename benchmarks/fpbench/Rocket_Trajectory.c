#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

float ex0(float Mf, float A) {
	float R = 6400000.0f;
	float G = 6.67428e-11f;
	float Mt = 5.9736e+24f;
	float dt = 0.1f;
	float T = 24.0f * 3600.0f;
	float nombrepas = T / dt;
	float r0 = (400.0f * 10000.0f) + R;
	float vr0 = 0.0f;
	float teta0 = 0.0f;
	float viss = sqrtf(((G * Mt) / r0));
	float vteta0 = viss / r0;
	float rf = R;
	float vrf = 0.0f;
	float tetaf = 0.0f;
	float vl = sqrtf(((G * Mt) / R));
	float vlrad = vl / r0;
	float vtetaf = 1.1f * vlrad;
	float t_i = 0.0f;
	float mf_i = 0.0f;
	float u1_i = 0.0f;
	float u3_i = 0.0f;
	float w1_i = 0.0f;
	float w3_i = 0.0f;
	float u2_i = 0.0f;
	float u4_i = 0.0f;
	float w2_i = 0.0f;
	float w4_i = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float i = 1.0f;
	float u1_im1 = r0;
	float u2_im1 = vr0;
	float u3_im1 = teta0;
	float u4_im1 = vteta0;
	float w1_im1 = rf;
	float w2_im1 = vrf;
	float w3_im1 = tetaf;
	float w4_im1 = vtetaf;
	float t_im1 = 0.0f;
	float mf_im1 = Mf;
	int tmp = i < 2000000.0f;
	while (tmp) {
		t_i = t_im1 + dt;
		mf_i = mf_im1 - (A * t_im1);
		u1_i = (u2_im1 * dt) + u1_im1;
		u3_i = (u4_im1 * dt) + u3_im1;
		w1_i = (w2_im1 * dt) + w1_im1;
		w3_i = (w4_im1 * dt) + w3_im1;
		u2_i = ((-G * (Mt / (u1_im1 * u1_im1))) * dt) + ((u1_im1 * u4_im1) * (u4_im1 * dt));
		u4_i = ((-2.0f * (u2_im1 * (u4_im1 / u1_im1))) * dt) + u4_im1;
		float tmp_1;
		if (mf_im1 > 0.0f) {
			tmp_1 = ((A * w2_im1) / (Mf - (A * t_im1))) * dt;
		} else {
			tmp_1 = 0.0f;
		}
		w2_i = (((-G * (Mt / (w1_im1 * w1_im1))) * dt) + ((w1_im1 * w4_im1) * (w4_im1 * dt))) + (tmp_1 + w2_im1);
		float tmp_2;
		if (mf_im1 > 0.0f) {
			tmp_2 = A * ((w4_im1 / (Mf - (A * t_im1))) * dt);
		} else {
			tmp_2 = 0.0f;
		}
		w4_i = ((-2.0f * (w2_im1 * (w4_im1 / w1_im1))) * dt) + (tmp_2 + w4_im1);
		x = u1_i * cosf(u3_i);
		y = u1_i * sinf(u3_i);
		i = i + 1.0f;
		u1_im1 = u1_i;
		u2_im1 = u2_i;
		u3_im1 = u3_i;
		u4_im1 = u4_i;
		w1_im1 = w1_i;
		w2_im1 = w2_i;
		w3_im1 = w3_i;
		w4_im1 = w4_i;
		t_im1 = t_i;
		mf_im1 = mf_i;
		tmp = i < 2000000.0f;
	}
	return x;
}

