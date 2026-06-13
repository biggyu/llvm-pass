#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double x) {
	return exp((a * x)) - 1.0;
}

