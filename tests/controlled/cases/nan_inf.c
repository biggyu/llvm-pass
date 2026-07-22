#include <math.h>
#include <stdio.h>

int main() {
    volatile double z = 0.0, o = 1.0;
    volatile double nan = z / z;
    volatile double inf = o / z;
    printf("%g %g\n", nan, inf);
    return 0;
}