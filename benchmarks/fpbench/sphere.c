#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x, double r, double lat, double lon) {
	double sinLat = sin(lat);
	double cosLon = cos(lon);
	return x + ((r * sinLat) * cosLon);
}

