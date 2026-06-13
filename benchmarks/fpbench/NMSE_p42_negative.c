#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double b, double c) {
	return (-b - sqrt(((b * b) - (4.0 * (a * c))))) / (2.0 * a);
}

