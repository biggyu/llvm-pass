#include <stdio.h>

// Uses int arithmetic
int add_ints(int a, int b) {
    int sum = a + b;
    int prod = a * b;
    return sum + prod; // combine results
}

// Uses float arithmetic
float mix_float(float x, float y) {
    float diff = x - y;
    float div  = (y != 0.0f) ? (x / y) : 0.0f;
    return diff + div;
}

// Uses double arithmetic
double poly_double(double x) {
    // x^3 - 2x^2 + 0.5x + 7
    return x * x * x - 2.0 * x * x + 0.5 * x + 7.0;
}

int main(void) {
    int i = 6, j = 4;
    float f1 = 3.5f, f2 = 1.25f;
    double d1 = 2.0;

    int int_result = add_ints(i, j);
    float float_result = mix_float(f1, f2);
    double double_result = poly_double(d1);

    // Mix types in one expression (shows implicit casts)
    double mixed = (double)i + d1 * (double)f1;

    printf("int_result   = %d\n", int_result);
    printf("float_result = %f\n", float_result);
    printf("double1      = %lf\n", double_result);
    printf("mixed        = %lf\n", mixed);

    return 0;
}
