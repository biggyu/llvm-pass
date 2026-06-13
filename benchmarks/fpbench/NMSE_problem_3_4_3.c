#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double eps) {
	return log(((1.0 - eps) / (1.0 + eps)));
}

