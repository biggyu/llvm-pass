#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double N) {
	return atan((N + 1.0)) - atan(N);
}

