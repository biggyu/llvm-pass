#include <math.h>
#include <stdio.h>

int main() {
    volatile double acc = 0;
    double a = 2.0, b = 3.0, c = 1.0;
    for (double x = 0.1; x < 10.0; x += 0.1) {
        acc += a * x * x + b * x + c * x;
    }
    printf("%g\n", acc);
    return 0;
}