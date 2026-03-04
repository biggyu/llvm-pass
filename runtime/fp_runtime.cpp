#include <stdio.h>
#include <math.h>

float addf(float a, float b) {
    float x = a + b;
    float y = x + 10.25f;
    return y;
}

double addd(double a, double b) {
    double s = a + b;
    double t = s + 3.1415;
    return t;
}

void TwoSum_D(double a, double b, double *x, double *dx) {
    *x = a + b;
    double bp = *x - a;
    double ap = *x - bp;
    double da = a - ap;
    double db = b - bp;
    *dx = da + db;
}
void TwoSum_F(float a, float b, float *x, float *dx) {
    *x = a + b;
    float bp = *x - a;
    float ap = *x - bp;
    float da = a - ap;
    float db = b - bp;
    *dx = da + db;
}

void TwoProd(double a, double b, double *x, double *dx) {
    *x = a * b;
    *dx = fma(a, b, -(*x));
    // *dx = std::fma(a, b, -x);
}