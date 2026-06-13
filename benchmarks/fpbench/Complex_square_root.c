#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double re, double im) {
	return 0.5 * sqrt((2.0 * (sqrt(((re * re) + (im * im))) + re)));
}

