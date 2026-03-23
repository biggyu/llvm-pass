#pragma once

#ifdef __cplusplus
extern "C" {
#endif

float addf(float a, float b);
double addd(double a, double b);

void TwoSumF(float a, float b, float *x, float *dx);
void TwoSumD(double a, double b, double *x, double *dx);

void TwoProdF(float a, float b, float *x, float *dx);
void TwoProdD(double a, double b, double *x, double *dx);

#ifdef __cplusplus
}
#endif