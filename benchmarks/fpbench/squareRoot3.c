#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	double tmp;
	if (x < 1e-5) {
		tmp = 1.0 + (0.5 * x);
	} else {
		tmp = sqrt((1.0 + x));
	}
	return tmp;
}

