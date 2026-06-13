#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double u) {
	return -((u * u) * u) / 6.0;
}

