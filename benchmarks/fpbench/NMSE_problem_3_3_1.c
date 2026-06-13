#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x) {
	return (1.0 / (x + 1.0)) - (1.0 / x);
}

