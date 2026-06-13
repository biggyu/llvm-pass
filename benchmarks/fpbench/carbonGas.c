#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double v) {
	double p = 35000000.0;
	double a = 0.401;
	double b = 4.27e-5;
	double t = 300.0;
	double n = 1000.0;
	double k = 1.3806503e-23;
	return ((p + ((a * (n / v)) * (n / v))) * (v - (n * b))) - ((k * n) * t);
}

