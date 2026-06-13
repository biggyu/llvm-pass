#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x1, double x2, double x3, double x4, double x5, double x6) {
	return (((((((x1 * x4) * (((((-x1 + x2) + x3) - x4) + x5) + x6)) + ((x2 * x5) * (((((x1 - x2) + x3) + x4) - x5) + x6))) + ((x3 * x6) * (((((x1 + x2) - x3) + x4) + x5) - x6))) - ((x2 * x3) * x4)) - ((x1 * x3) * x5)) - ((x1 * x2) * x6)) - ((x4 * x5) * x6);
}

