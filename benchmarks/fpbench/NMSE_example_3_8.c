#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double N) {
	return (((N + 1.0) * log((N + 1.0))) - (N * log(N))) - 1.0;
}

