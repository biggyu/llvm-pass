#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double lat1, double lat2, double lon1, double lon2) {
	double dLon = lon2 - lon1;
	double s_lat1 = sin(lat1);
	double c_lat1 = cos(lat1);
	double s_lat2 = sin(lat2);
	double c_lat2 = cos(lat2);
	double s_dLon = sin(dLon);
	double c_dLon = cos(dLon);
	return atan(((c_lat2 * s_dLon) / ((c_lat1 * s_lat2) - ((s_lat1 * c_lat2) * c_dLon))));
}

