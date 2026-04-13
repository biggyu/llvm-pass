#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//TODO: merge into below
// void PropUnFloatError(float a, float da, float *x, float *dx);
// void PropBinFloatError(float a, float da, float b, float db, float *x, float *dx);
// void PropUnDoubleError(double a, double da, double *x, double *dx);
// void PropBinDoubleError(double a, double da, double b, double db, double *x, double *dx);

void PropSinFError(float a, float da, float *x, float *dx);
void PropSinDError(double a, double da, double *x, double *dx);
// void PropsinfFError(float a, float da, float *x, float *dx);
// void PropsinfDError(double a, double da, double *x, double *dx);
void PropCosFError(float a, float da, float *x, float *dx);
void PropCosDError(double a, double da, double *x, double *dx);
// void PropcosfFError(float a, float da, float *x, float *dx);
// void PropcosfDError(double a, double da, double *x, double *dx);
void PropTanFError(float a, float da, float *x, float *dx);
void PropTanDError(double a, double da, double *x, double *dx);
void PropAsinFError(float a, float da, float *x, float *dx);
void PropAsinDError(double a, double da, double *x, double *dx);
void PropAcosFError(float a, float da, float *x, float *dx);
void PropAcosDError(double a, double da, double *x, double *dx);
void PropAtanFError(float a, float da, float *x, float *dx);
void PropAtanDError(double a, double da, double *x, double *dx);
void PropLogFError(float a, float da, float *x, float *dx);
void PropLogDError(double a, double da, double *x, double *dx);
void PropExpFError(float a, float da, float *x, float *dx);
void PropExpDError(double a, double da, double *x, double *dx);
void PropPowFError(float a, float da, float b, float db, float *x, float *dx);
void PropPowDError(double a, double da, double b, double db, double *x, double *dx);
// void PropexpfFError(float a, float da, float *x, float *dx);
// void PropexpfDError(double a, double da, double *x, double *dx);
void PropFabsFError(float a, float da, float *x, float *dx);
void PropFabsDError(double a, double da, double *x, double *dx);
// void PropfabsfFError(float a, float da, float *x, float *dx);
// void PropfabsfDError(double a, double da, double *x, double *dx);

#ifdef __cplusplus
}
#endif