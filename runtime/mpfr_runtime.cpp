#include "mpfr_runtime.h"
#include <gmp.h>
#include <mpfr.h>
#include <cmath>

// void PropUnFloatError(float a, float da, float *x, float *dx) {
    
// }
// void PropBinFloatError(float a, float da, float b, float db, float *x, float *dx) {
    
// }
// void PropUnDoubleError(double a, double da, double *x, double *dx) {

// }
// void PropBinDoubleError(double a, double da, double b, double db, double *x, double *dx) {

// }
void PropSinFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_flt(mx, MPFR_RNDN);
    double dprec = sinf(a);

    *x = dprec;
    *dx = (hprec - dprec) + cosf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropSinDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = sin(a);

    *x = dprec;
    *dx = (hprec - dprec) + cos(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
// void PropsinfFError(float a, float da, float *x, float *dx);
// void PropsinfDError(double a, double da, double *x, double *dx);
void PropCosFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = cosf(a);

    *x = dprec;
    *dx = (hprec - dprec) - sinf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropCosDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = cos(a);

    *x = dprec;
    *dx = (hprec - dprec) - sin(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
// void PropcosfFError(float a, float da, float *x, float *dx);
// void PropcosfDError(double a, double da, double *x, double *dx);
void PropTanFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_tan(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = tanf(a);

    *x = dprec;
    *dx = (hprec - dprec) + powf((1 / cosf(a)), 2) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropTanDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_tan(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = tan(a);

    *x = dprec;
    *dx = (hprec - dprec) + pow((1 / cos(a)), 2) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropAsinFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_asin(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = asinf(a);

    *x = dprec;
    *dx = (hprec - dprec) + 1 / sqrtf(1 - powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropAsinDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_asin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = asin(a);

    *x = dprec;
    *dx = (hprec - dprec) + 1 / sqrt(1 - pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropAcosFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_acos(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = acosf(a);

    *x = dprec;
    *dx = (hprec - dprec) - 1 / sqrtf(1 - powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropAcosDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_acos(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = acos(a);

    *x = dprec;
    *dx = (hprec - dprec) - 1 / sqrt(1 - pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropAtanFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_atan(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = atanf(a);

    *x = dprec;
    *dx = (hprec - dprec) + 1 / (1 + powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropAtanDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_atan(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = atan(a);

    *x = dprec;
    *dx = (hprec - dprec) + 1 / (1 + pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropLogFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = logf(a);

    *x = dprec;
    *dx = (hprec - dprec) + (1 / a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropLogDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = log(a);

    *x = dprec;
    *dx = (hprec - dprec) + (1 / a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropExpFError(float a, float da, float *x, float *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = expf(a);

    *x = dprec;
    *dx = (hprec - dprec) + expf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}
void PropExpDError(double a, double da, double *x, double *dx) {
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = exp(a);

    *x = dprec;
    *dx = (hprec - dprec) + exp(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
}   
void PropPowFError(float a, float da, float b, float db, float *x, float *dx) {
    mpfr_t ma, mb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_set_flt(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = powf(a, b);

    *x = dprec;
    *dx = (hprec - dprec) + b * powf(a, b - 1) * da + powf(a, b) * logf(b) * db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mx);
}
void PropPowDError(double a, double da, double b, double db, double *x, double *dx) {
    mpfr_t ma, mb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_set_d(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = pow(a, b);

    *x = dprec;
    *dx = (hprec - dprec) + b * pow(a, b - 1) * da + pow(a, b) * log(b) * db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mx);
}   
// void PropexpfFError(float a, float da, float *x, float *dx);
// void PropexpfDError(double a, double da, double *x, double *dx);
void PropFabsFError(float a, float da, float *x, float *dx) {
    *x = fabsf(a);
    if (a > 0.0f) {
        *dx = da;
    }
    else if (a < 0.0f) {
        *dx = -da;
    }
    else {
        *dx = fabsf(da);
    }
}
void PropFabsDError(double a, double da, double *x, double *dx) {
    *x = fabs(a);
    if (a > 0.0) {
        *dx = da;
    }
    else if (a < 0.0) {
        *dx = -da;
    }
    else {
        *dx = fabs(da);
    }
}   