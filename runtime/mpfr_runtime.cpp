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
fp_entryF PropSinFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_flt(mx, MPFR_RNDN);
    double dprec = sinf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + cosf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropSinDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = sin(a);

    result.value = dprec;
    result.error = (hprec - dprec) + cos(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
// void PropsinfFError(float a, float da, float *x, float *dx);
// void PropsinfDError(double a, double da, double *x, double *dx);
fp_entryF PropCosFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = cosf(a);

    result.value = dprec;
    result.error = (hprec - dprec) - sinf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropCosDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = cos(a);

    result.value = dprec;
    result.error = (hprec - dprec) - sin(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
// void PropcosfFError(float a, float da, float *x, float *dx);
// void PropcosfDError(double a, double da, double *x, double *dx);
fp_entryF PropTanFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_tan(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = tanf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + powf((1 / cosf(a)), 2) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropTanDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_tan(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = tan(a);

    result.value = dprec;
    result.error = (hprec - dprec) + pow((1 / cos(a)), 2) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAsinFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_asin(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = asinf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + 1 / sqrtf(1 - powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAsinDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_asin(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = asin(a);

    result.value = dprec;
    result.error = (hprec - dprec) + 1 / sqrt(1 - pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAcosFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_acos(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = acosf(a);

    result.value = dprec;
    result.error = (hprec - dprec) - 1 / sqrtf(1 - powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAcosDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_acos(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = acos(a);

    result.value = dprec;
    result.error = (hprec - dprec) - 1 / sqrt(1 - pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAtanFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_atan(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = atanf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + 1 / (1 + powf(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAtanDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_atan(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = atan(a);

    result.value = dprec;
    result.error = (hprec - dprec) + 1 / (1 + pow(a, 2)) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropLogFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = logf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + (1 / a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropLogDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = log(a);

    result.value = dprec;
    result.error = (hprec - dprec) + (1 / a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropExpFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = expf(a);

    result.value = dprec;
    result.error = (hprec - dprec) + expf(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropExpDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = exp(a);

    result.value = dprec;
    result.error = (hprec - dprec) + exp(a) * da;

    mpfr_clear(ma);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropPowFError(float a, float da, float b, float db) {
    fp_entryF result;
    mpfr_t ma, mb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_set_flt(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);
    float hprec = mpfr_get_flt(mx, MPFR_RNDN);
    float dprec = powf(a, b);

    result.value = dprec;
    result.error = (hprec - dprec) + b * powf(a, b - 1) * da + powf(a, b) * logf(b) * db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropPowDError(double a, double da, double b, double db) {
    fp_entryD result;
    mpfr_t ma, mb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_set_d(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = pow(a, b);

    result.value = dprec;
    result.error = (hprec - dprec) + b * pow(a, b - 1) * da + pow(a, b) * log(b) * db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mx);
    return result;
}   
// void PropexpfFError(float a, float da, float *x, float *dx);
// void PropexpfDError(double a, double da, double *x, double *dx);
fp_entryF PropFabsFError(float a, float da) {
    fp_entryF result;
    result.value = fabsf(a);
    if (a > 0.0f) {
        result.error = da;
    }
    else if (a < 0.0f) {
        result.error = -da;
    }
    else {
        result.error = fabsf(da);
    }
    return result;
}
fp_entryD PropFabsDError(double a, double da) {
    fp_entryD result;
    result.value = fabs(a);
    if (a > 0.0) {
        result.error = da;
    }
    else if (a < 0.0) {
        result.error = -da;
    }
    else {
        result.error = fabs(da);
    }
    return result;
}   