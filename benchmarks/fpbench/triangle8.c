#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double b, double c) {
	double s = ((a + b) + c) / 2.0;
	return sqrt((((s * (s - a)) * (s - b)) * (s - c)));
}

