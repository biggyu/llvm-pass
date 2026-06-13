#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x1, double x2) {
	double a = ((x1 * x1) + x2) - 11.0;
	double b = (x1 + (x2 * x2)) - 7.0;
	return (a * a) + (b * b);
}

