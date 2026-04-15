#include <stdio.h>
#include <math.h>

int main(void) {
    float  f1 = 1.0f, f2 = 1e-7f;
    double a = 10.5, b = 3.2;

    double addd = a + b;
    double subd = a - b;
    double muld = a * b;
    // double result1d = muld - addd + subd;
    // double result2d = -1 * a / b;
    double result3d = sqrt(muld) * sqrt(addd) / sqrt(subd);

    float addf = f1 + f2;
    float subf = f1 - f2;
    float mulf = f1 * f2;
    // float result1f = mulf - addf - subf;
    // float result2f = -1 * f1 / f2;
    float result3f = sqrtf(mulf) * sqrtf(addf) / sqrtf(subf);

    printf("add = %f\n", addd);
    printf("sub = %f\n", subd);
    printf("mul = %f\n", muld);
    // printf("result1 = %f\n", result1d);
    // printf("result2 = %f\n", result2d);
    printf("result3 = %f\n", result3d);

    printf("add = %f\n", addf);
    printf("sub = %f\n", subf);
    printf("mul = %f\n", mulf);
    // printf("result1 = %f\n", result1f);
    // printf("result2 = %f\n", result2f);
    printf("result3 = %f\n", result3f);
    return 0;
}