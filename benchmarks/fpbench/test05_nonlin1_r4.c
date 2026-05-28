#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	double r1 = x - 1.0;
	double r2 = x * x;
	return r1 / (r2 - 1.0);
}

