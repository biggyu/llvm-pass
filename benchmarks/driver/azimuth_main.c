// #include <fenv.h>
// #include <math.h>
// #include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "../../include/fp_debug.h"
// #define TRUE 1
// #define FALSE 0

#ifdef __cplusplus
extern "C" {
#endif

double ex0(double lat1, double lat2, double lon1, double lon2);

#ifdef __cplusplus
}
#endif

int main() {
	double lat1 = 1, lat2 = 2, lon1 = 3, lon2 = 4;
	printf("%f\n", ex0(lat1, lat2, lon1, lon2));

    report_debug_summary();

	return 0;
}
