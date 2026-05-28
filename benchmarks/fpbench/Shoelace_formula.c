#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x1, double y1, double x2, double y2, double x3, double y3) {
	double s1 = (x1 * y2) - (y1 * x2);
	double s2 = (x2 * y3) - (y2 * x3);
	double s3 = (x3 * y1) - (y3 * x1);
	return 0.5 * ((s1 + s2) + s3);
}

