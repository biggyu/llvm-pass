#include <math.h>
#include <stdio.h>

int main() {
    volatile double acc = 0;
    for (double x = 1e-8; x < 1e-2; x *= 1.5) {
        acc += sin(x);
    }
    printf("%g\n", acc);
    return 0;
}