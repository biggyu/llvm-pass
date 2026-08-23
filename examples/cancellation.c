#include <math.h>
#include <stdio.h>

double f(double x) {
    return sqrt(x + 1.0) - sqrt(x);
}
double f1(double x) {
    return ((1 - cos(x)) / (x * x));
}
int main() {
    volatile double x = 1e100;
    printf("%g\n", f(x));
    volatile double x1 = 1e200;
    printf("%g\n", f1(x1));
    return 0;
}
// #include <gmp.h>
// #include <math.h>
// #include <mpfr.h>
// #include <stdio.h>

// int main() {
//     mpfr_t mx, mx1, msq1, msq, mr;
//     mpfr_init2(mx, 400);
//     mpfr_init2(mx1, 400);
//     mpfr_init2(msq1, 400);
//     mpfr_init2(msq, 400);
//     mpfr_init2(mr, 400);

//     mpfr_set_d(mx, 1e100, MPFR_RNDN);
//     mpfr_add_ui(mx1, mx, 1, MPFR_RNDN);
//     mpfr_sqrt(msq1, mx1, MPFR_RNDN);
//     mpfr_sqrt(msq, mx, MPFR_RNDN);
//     mpfr_sub(mr, msq1, msq, MPFR_RNDN);
//     mpfr_printf("true value: %.30Rg\n", mr);
//     double result = mpfr_get_d(mr, MPFR_RNDN);
//     printf("as double: %g\n", result);
    
//     mpfr_set_d(mx, 1e200, MPFR_RNDN);
//     mpfr_cos(msq, mx, MPFR_RNDN);
//     mpfr_mul(msq1, mx, mx, MPFR_RNDN);
//     mpfr_si_sub(mx1, 1.0, msq, MPFR_RNDN);
//     mpfr_div(mr, mx1, msq1, MPFR_RNDN);
//     mpfr_printf("true value: %.30Rg\n", mr);
//     result = mpfr_get_d(mr, MPFR_RNDN);
//     printf("as double: %g\n", result);
    
//     mpfr_clear(mx);
//     mpfr_clear(mx1);
//     mpfr_clear(mr);
//     mpfr_clear(msq1);
//     mpfr_clear(msq);
//     return 0;
// }