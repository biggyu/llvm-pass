#include "mpfr_runtime.h"
#include <gmp.h>
#include <mpfr.h>
#include <cmath>

fp_entryF PropSinFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_cos(mt, ma, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);

    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_set_d(mx, MPFR_RNDN);
    float dprec = sinf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropSinDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_cos(mt, ma, MPFR_RNDN);
    mpfr_sin(mx, ma, MPFR_RNDN);

    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = sin(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
// void PropsinfFError(float a, float da, float *x, float *dx);
// void PropsinfDError(double a, double da, double *x, double *dx);
fp_entryF PropCosFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_sin(mt, ma, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);

    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = cosf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) - taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropCosDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_sin(mt, ma, MPFR_RNDN);
    mpfr_cos(mx, ma, MPFR_RNDN);

    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = cos(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) - taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
// void PropcosfFError(float a, float da, float *x, float *dx);
// void PropcosfDError(double a, double da, double *x, double *dx);
fp_entryF PropTanFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);

    mpfr_tan(mx, ma, MPFR_RNDN);
    mpfr_mul(mt, mx, mx, MPFR_RNDN);

    mpfr_ui(mt, mt, 1, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = tanf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropTanDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);

    mpfr_tan(mx, ma, MPFR_RNDN);
    mpfr_mul(mt, mx, mx, MPFR_RNDN);

    mpfr_ui(mt, mt, 1, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = tan(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAsinFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);

    mpfr_mul(mt, ma, ma, MPFR_RNDN);
    mpfr_ui_sub(mt, 1, mt, RNDN);
    mpfr_sqrt(mt, mt, MPFR_RNDN);
    mpfr_ui_div(mt, 1, mt, MPFR_RNDN);

    mpfr_asin(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = asinf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAsinDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);

    mpfr_mul(mt, ma, ma, MPFR_RNDN);
    mpfr_ui_sub(mt, 1, mt, RNDN);
    mpfr_sqrt(mt, mt, MPFR_RNDN);
    mpfr_ui_div(mt, 1, mt, MPFR_RNDN);
    mpfr_asin(mx, ma, MPFR_RNDN);

    mpfr_asin(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = asin(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAcosFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    
    mpfr_mul(mt, ma, ma, MPFR_RNDN);
    mpfr_ui_sub(mt, 1, mt, RNDN);
    mpfr_sqrt(mt, mt, MPFR_RNDN);
    mpfr_ui_div(mt, 1, mt, MPFR_RNDN);
    mpfr_neg(mt, mt, MPFR_RNDN);
    
    mpfr_acos(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = acosf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) - taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAcosDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    
    mpfr_mul(mt, ma, ma, MPFR_RNDN);
    mpfr_ui_sub(mt, 1, mt, RNDN);
    mpfr_sqrt(mt, mt, MPFR_RNDN);
    mpfr_ui_div(mt, 1, mt, MPFR_RNDN);
    mpfr_neg(mt, mt, MPFR_RNDN);

    mpfr_acos(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = acos(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) - taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropAtanFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    
    mpfr_mul(mt, ma, ma, MPFR_RNDN);
    mpfr_add_ui(mt, mt, 1, RNDN);
    mpfr_ui_div(mt, 1, mt, MPFR_RNDN);
    mpfr_neg(mt, mt, MPFR_RNDN);

    mpfr_atan(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = atanf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropAtanDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_atan(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = atan(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropLogFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_ui_div(md, 1, ma, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = logf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropLogDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_ui_div(md, 1, ma, MPFR_RNDN);
    mpfr_log(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = log(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropExpFError(float a, float da) {
    fp_entryF result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_exp(mt, ma, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = expf(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropExpDError(double a, double da) {
    fp_entryD result;
    mpfr_t ma, mt, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mt, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_exp(mt, ma, MPFR_RNDN);
    mpfr_exp(mx, ma, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = exp(a);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + taylor * (double)da;

    mpfr_clear(ma);
    mpfr_clear(mt);
    mpfr_clear(mx);
    return result;
}   
fp_entryF PropPowFError(float a, float da, float b, float db) {
    fp_entryF result;
    mpfr_t ma, mb, mda, mdb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mda, 200);
    mpfr_init2(mdb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_flt(ma, a, MPFR_RNDN);
    mpfr_set_flt(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);

    mpfr_div(mda, mx, ma, MPFR_RNDN);
    mpfr_mul(mda, mda, mb, MPFR_RNDN);

    mpfr_log(mdb, ma, MPFR_RNDN);
    mpfr_mul(mdb, mdb, ma, MPFR_RNDN);
    
    double dda = mpfr_get_d(mda, MPFR_RNDN);
    double ddb = mpfr_get_d(mdb, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    float dprec = powf(a, b);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + dda * (double)da + ddb * (double)db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mda);
    mpfr_clear(mdb);
    mpfr_clear(mx);
    return result;
}
fp_entryD PropPowDError(double a, double da, double b, double db) {
    fp_entryD result;
    mpfr_t ma, mb, mda, mdb, mx;
    mpfr_init2(ma, 200);
    mpfr_init2(mb, 200);
    mpfr_init2(mda, 200);
    mpfr_init2(mdb, 200);
    mpfr_init2(mx, 200);
    
    mpfr_set_d(ma, a, MPFR_RNDN);
    mpfr_set_d(mb, b, MPFR_RNDN);
    mpfr_pow(mx, ma, mb, MPFR_RNDN);

    mpfr_div(mda, mx, ma, MPFR_RNDN);
    mpfr_mul(mda, mda, mb, MPFR_RNDN);

    mpfr_log(mdb, ma, MPFR_RNDN);
    mpfr_mul(mdb, mdb, ma, MPFR_RNDN);
    
    double dda = mpfr_get_d(mda, MPFR_RNDN);
    double ddb = mpfr_get_d(mdb, MPFR_RNDN);
    
    double taylor = mpfr_get_d(mt, MPFR_RNDN);
    double hprec = mpfr_get_d(mx, MPFR_RNDN);
    double dprec = pow(a, b);

    result.value = dprec;
    result.error = (hprec - (double)dprec) + dda * (double)da + ddb * (double)db;

    mpfr_clear(ma);
    mpfr_clear(mb);
    mpfr_clear(mda);
    mpfr_clear(mdb);
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