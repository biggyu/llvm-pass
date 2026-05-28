#define _USE_MATH_DEFINES
#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(int64_t n) {
	double dppi = (double) M_PI;
	double h = dppi / ((double) n);
	double t1 = 0.0;
	double t2 = 0.0;
	double s1 = 0.0;
	double t1_1 = t1;
	int64_t tmp = 1;
	int64_t i = tmp;
	int tmp_2 = i <= n;
	while (tmp_2) {
		double x = ((double) i) * h;
		float tmp_3 = 1.0f;
		float d1 = tmp_3;
		double t1_4 = x;
		int64_t tmp_5 = 1;
		int64_t k = tmp_5;
		int tmp_6 = k <= 5.0;
		while (tmp_6) {
			float tmp_7 = d1 * 2.0f;
			d1 = tmp_7;
			t1_4 = t1_4 + (sin((((double) d1) * x)) / ((double) d1));
			int64_t tmp_8 = k + 1;
			k = tmp_8;
			tmp_6 = k <= 5.0;
		}
		t2 = t1_4;
		double s0 = sqrt(((h * h) + ((t2 - t1_1) * (t2 - t1_1))));
		long double tmp_9 = ((long double) s1) + ((long double) s0);
		s1 = tmp_9;
		t1_1 = t2;
		int64_t tmp_10 = i + 1;
		i = tmp_10;
		tmp_2 = i <= n;
	}
	return s1;
}
