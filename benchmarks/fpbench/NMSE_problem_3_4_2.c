#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double b, double eps) {
	return (eps * (exp(((a + b) * eps)) - 1.0)) / ((exp((a * eps)) - 1.0) * (exp((b * eps)) - 1.0));
}

