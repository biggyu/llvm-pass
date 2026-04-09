#pragma once

#ifdef __cplusplus
extern "C" {
#endif

float addf(float a, float b);
double addd(double a, double b);

void PropSumFError(float a, float da, float b, float db, float *x, float *dx);
void PropSumDError(double a, double da, double b, double db, double *x, double *dx);
void PropProdFError(float a, float da, float b, float db, float *x, float *dx);
void PropProdDError(double a, double da, double b, double db, double *x, double *dx);

void TwoSumF(float a, float b, float *x, float *dx);
void TwoSumD(double a, double b, double *x, double *dx);

void TwoProdF(float a, float b, float *x, float *dx);
void TwoProdD(double a, double b, double *x, double *dx);

void report_fp_profile();

// void PropSumError(double a, double da, double b, double db, double& x, double& dx) {
//     TwoSum(a, b, x, dx);
//     dx = dx + da + db;
// }

// void TwoProd(double a, double b, double& x, double& dx) {
//     x = a * b;
//     dx = std::fma(a, b, -x);
// }

// void PropProdError(double a, double da, double b, double db, double& x, double& dx) {
//     TwoProd(a, b, x, dx);
//     dx = dx + a * db + b * da;
// }

#ifdef __cplusplus
}
#endif