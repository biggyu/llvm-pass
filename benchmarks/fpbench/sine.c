#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	return ((x - (((x * x) * x) / 6.0)) + (((((x * x) * x) * x) * x) / 120.0)) - (((((((x * x) * x) * x) * x) * x) * x) / 5040.0);
}

