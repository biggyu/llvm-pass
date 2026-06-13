#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double a, double b) {
	return (((333.75 * pow(b, 6.0)) + (pow(a, 2.0) * (((((11.0 * pow(a, 2.0)) * pow(b, 2.0)) - pow(b, 6.0)) - (121.0 * pow(b, 4.0))) - 2.0))) + (5.5 * pow(b, 8.0))) + (a / (2.0 * b));
}

