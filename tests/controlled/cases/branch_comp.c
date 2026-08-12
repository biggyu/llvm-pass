#include <stdio.h>

__attribute__((noinline))
double accumulate(double n, int iters) {
    double s = 0.0;
    for (int i = 0; i < iters; i++) {
        s += n;          // repeated addition accumulates residual
    }
    return s;
}

int main(int argc, char **argv) {
    int iters = (argc > 1) ? 10000 : 10000;
    double step = 0.1;                     // not exactly representable
    double sum = accumulate(step, iters);  // true value 1000.0, computed drifts
    // computed sum is slightly off from 1000.0; the accumulated residual
    // should be many ulps by now
    printf("%.20g\n", sum);
    if (sum <= 1000.0) return 1;            // predicate near the true value
    return 0;
}