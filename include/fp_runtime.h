#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct fp_entry_f {
    float value;
    float error;
};
struct fp_entry_d {
    double value;
    double error;
};

// float addf(float a, float b);
// double addd(double a, double b);

fp_entry_f PropSumFError(float a, float da, float b, float db);
fp_entry_d PropSumDError(double a, double da, double b, double db);
fp_entry_f PropProdFError(float a, float da, float b, float db);
fp_entry_d PropProdDError(double a, double da, double b, double db);
fp_entry_f PropDivFError(float a, float da, float b, float db);
fp_entry_d PropDivDError(double a, double da, double b, double db);
fp_entry_f PropSqrtFError(float a, float da);
fp_entry_d PropSqrtDError(double a, double da);

// void TwoSumF(float a, float b, float *x, float *dx);
// void TwoSumD(double a, double b, double *x, double *dx);

// void TwoProdF(float a, float b, float *x, float *dx);
// void TwoProdD(double a, double b, double *x, double *dx);

// void TwoDivF(float a, float b, float *x, float *dx);
// void TwoDivD(double a, double b, double *x, double *dx);

// void SquareRootF(float a, float *x, float *dx);
// void SquareRootD(double a, double *x, double *dx);

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