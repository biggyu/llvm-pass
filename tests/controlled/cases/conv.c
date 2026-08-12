#include <stdio.h>
__attribute__((noinline))
double accumulate(double n, int iters) {
    double s = 0.0;
    for (int i = 0; i < iters; i++) s += n;
    return s;
}

int main(void) {
    double v = accumulate(0.1, 800);   // true 3.0, computed 2.9999999999999996
    printf("%.20g\n", v);
    int truncated = (int)v;            // 2 (computed) vs 3 (corrected)
    return truncated;
}