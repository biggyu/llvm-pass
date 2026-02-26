#include <stdio.h>

int main(void) {
    // float  f1 = 1.0f, f2 = 1e-7f;
    double a = 10.5, b = 3.2;

    double add = a + b;
    double sub = a - b;
    double mul = a * b;
    double result1 = mul - add - sub;
    double result2 = -1 * a * b;

    printf("add = %f\n", add);
    printf("sub = %f\n", sub);
    printf("mul = %f\n", mul);
    printf("result1 = %f\n", result1);
    printf("result2 = %f\n", result2);
    return 0;
}