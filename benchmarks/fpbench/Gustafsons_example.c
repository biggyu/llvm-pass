#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	double q = fabs((x - sqrt(((x * x) + 1.0)))) - (1.0 / (x + sqrt(((x * x) + 1.0))));
	double y = q * q;
	double tmp;
	if (y == 0.0) {
		tmp = 1.0;
	} else {
		tmp = (exp(y) - 1.0) / x;
	}
	return tmp;
}