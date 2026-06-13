#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

double ex0(double x1, double x2);

#ifdef __cplusplus
}
#endif

int main() {
    double x1 = 1e3, x2 = 2;
    printf("%f\n", ex0(x1, x2));
    return 0;
}