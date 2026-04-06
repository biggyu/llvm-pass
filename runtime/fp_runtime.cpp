#include "fp_runtime.h"
#include <cmath>

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

void PropSumFError(float a, float da, float b, float db, float *x, float *dx) {
    TwoSumF(a, b, x, dx);
    *dx = *dx + da + db;
}
void PropSumDError(double a, double da, double b, double db, double *x, double *dx) {
    TwoSumD(a, b, x, dx);
    *dx = *dx + da + db;
}
void PropProdFError(float a, float da, float b, float db, float *x, float *dx) {
    TwoProdF(a, b, x, dx);
    *dx = *dx + a * db + b * da;
}
void PropProdDError(double a, double da, double b, double db, double *x, double *dx) {
    TwoProdD(a, b, x, dx);
    *dx = *dx + a * db + b * da;
}

void TwoSumF(float a, float b, float *x, float *dx) {
    *x = a + b;
    float bp = *x - a;
    float ap = *x - bp;
    float da = a - ap;
    float db = b - bp;
    *dx = da + db;
}
void TwoSumD(double a, double b, double *x, double *dx) {
    *x = a + b;
    double bp = *x - a;
    double ap = *x - bp;
    double da = a - ap;
    double db = b - bp;
    *dx = da + db;
}

void TwoProdF(float a, float b, float *x, float *dx) {
    *x = a * b;
    *dx = fma(a, b, -(*x));
    // *dx = std::fma(a, b, -x);
}
void TwoProdD(double a, double b, double *x, double *dx) {
    *x = a * b;
    *dx = fma(a, b, -(*x));
    // *dx = std::fma(a, b, -x);
}