// #include <fenv.h>
// #include <math.h>
// #include <stdint.h>
#include "stdio.h"
// #define TRUE 1
// #define FALSE 0

double ex0(double re, double im);

int main() {
	double re = 1e-8, im = 1e-6;
	printf("%f\n", ex0(re, im));
	return 0;
}
