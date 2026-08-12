#include <math.h>
#include <stdio.h>

int main() {
    volatile double acc = 0;
    volatile double one = 1.0;
    for (double x = 0; x < 1000; x++) {
        acc += log(one);
    }
    printf("%g\n", acc);
    return 0;
}