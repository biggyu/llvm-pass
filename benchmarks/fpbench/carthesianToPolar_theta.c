#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x, double y) {
	double pi = 3.14159265359;
	double radiant = atan((y / x));
	return radiant * (180.0 / pi);
}

