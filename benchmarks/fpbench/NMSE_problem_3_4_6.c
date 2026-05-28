#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x, double n) {
	return pow((x + 1.0), (1.0 / n)) - pow(x, (1.0 / n));
}

