#include <math.h>
#include <stdio.h>

int main() {
    volatile double acc = 0;
    for (double x = 1e10; x < 1e16; x *= 10) {
        double r = sqrt(x + 1.0) - sqrt(x);
        acc += r;
    }
    printf("%g\n", acc);
    return 0;
}