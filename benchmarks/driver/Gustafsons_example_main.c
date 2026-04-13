// #include <fenv.h>
// #include <math.h>
// #include <stdint.h>
#include <stdio.h>
// #define TRUE 1
// #define FALSE 0

double ex0(double x);

int main() {
	double x = 1e8;
	printf("%f\n", ex0(x));
	return 0;
}