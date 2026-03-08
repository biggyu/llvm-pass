#pragma once

#ifdef __cplusplus
extern "C" {
#endif

float  addf(float a, float b);
double addd(double a, double b);

void TwoSum_F(float a, float b, float *x, float *dx);
void TwoSum_D(double a, double b, double *x, double *dx);

// void TwoProd_f(float a, float b, float *p, float *e);
void TwoProd(double a, double b, double *x, double *dx);

void FP_Store(void* addr, double *x, double *dx);

double FP_Load(void* addr, double *x, double *dx);

#ifdef __cplusplus
}
#endif