#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x1, double x2, double x3) {
	return ((-(x1 * x2) - ((2.0 * x2) * x3)) - x1) - x3;
}

