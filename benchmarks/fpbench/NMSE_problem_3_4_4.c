#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	return sqrt(((exp((2.0 * x)) - 1.0) / (exp(x) - 1.0)));
}

