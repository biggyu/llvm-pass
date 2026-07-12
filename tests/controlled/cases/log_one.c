#include <math.h>
#include <stdio.h>

int main() {
    volatile double acc = 0;
    for (double x = 0; x < 1000; x++) {
        acc += log(1.0);
    }
    printf("%g\n", acc);
    return 0;
}