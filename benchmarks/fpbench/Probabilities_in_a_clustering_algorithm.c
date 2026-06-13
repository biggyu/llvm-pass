#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double cp, double cn, double t, double s) {
	return (pow((1.0 / (1.0 + exp(-s))), cp) * pow((1.0 - (1.0 / (1.0 + exp(-s)))), cn)) / (pow((1.0 / (1.0 + exp(-t))), cp) * pow((1.0 - (1.0 / (1.0 + exp(-t)))), cn));
}

