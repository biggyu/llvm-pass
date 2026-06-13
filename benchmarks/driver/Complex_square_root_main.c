#include <stdio.h>
#include "../../include/fp_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

double ex0(double re, double im);

#ifdef __cplusplus
}
#endif

int main() {
    double re = 1e4, im = 2;
    printf("%f\n", ex0(re, im));

    report_debug_summary();

    return 0;
}