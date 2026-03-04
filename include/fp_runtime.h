#pragma once

#ifdef __cplusplus
extern "C" {
#endif

float  addf(float a, float b);
double addd(double a, double b);

void TwoSum_f(float a, float b, float *s, float *e);
void TwoSum_d(double a, double b, double *s, double *e);

// void TwoProd_f(float a, float b, float *p, float *e);
void TwoProd(double a, double b, double *p, double *e);

#ifdef __cplusplus
}
#endif