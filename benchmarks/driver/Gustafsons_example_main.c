// #include <fenv.h>
// #include <math.h>
// #include <stdint.h>
#include <stdio.h>
#include "../../include/fp_debug.h"
// #define TRUE 1
// #define FALSE 0

#ifdef __cplusplus
extern "C" {
#endif

double ex0(double x);

#ifdef __cplusplus
}
#endif

int main() {
	double x = 1e8;
	printf("%f\n", ex0(x));

    report_debug_summary();

	return 0;
}