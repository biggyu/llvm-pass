#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double c) {
	double a = 3.0;
	double b = 3.5;
	double discr = (b * b) - ((a * c) * 4.0);
	double tmp_1;
	if (((b * b) - (a * c)) > 10.0) {
		double tmp_2;
		if (b > 0.0) {
			tmp_2 = (c * 2.0) / (-b - sqrt(discr));
		} else if (b < 0.0) {
			tmp_2 = (-b + sqrt(discr)) / (a * 2.0);
		} else {
			tmp_2 = (-b + sqrt(discr)) / (a * 2.0);
		}
		tmp_1 = tmp_2;
	} else {
		tmp_1 = (-b + sqrt(discr)) / (a * 2.0);
	}
	return tmp_1;
}

