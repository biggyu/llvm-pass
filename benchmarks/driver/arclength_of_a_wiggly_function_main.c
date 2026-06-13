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

double ex0(int64_t n);

#ifdef __cplusplus
}
#endif

int main() {
	int64_t n = 4;
	printf("%f\n", ex0(n));

    report_debug_summary();

	return 0;
}