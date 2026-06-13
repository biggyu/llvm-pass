#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double b2, double c) {
	return (-b2 + sqrt(((b2 * b2) - (a * c)))) / a;
}

