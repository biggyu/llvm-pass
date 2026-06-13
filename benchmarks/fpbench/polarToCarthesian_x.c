#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double radius, double theta) {
	double pi = 3.14159265359;
	double radiant = theta * (pi / 180.0);
	return radius * cos(radiant);
}

