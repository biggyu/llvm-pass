#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x, double y) {
	double t = x * y;
	return (t - 1.0) / ((t * t) - 1.0);
}

