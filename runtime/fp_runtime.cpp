#include "fp_runtime.h"
#include <cmath>
#ifdef ENABLE_RUNTIME_TIME
#include <iostream>
#include <chrono>
using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

static uint64_t  propsumf_calls = 0;
static uint64_t  propsumd_calls = 0;
static uint64_t  propprodf_calls = 0;
static uint64_t  propprodd_calls = 0;
static uint64_t  propsumf_ns = 0;
static uint64_t  propsumd_ns = 0;
static uint64_t  propprodf_ns = 0;
static uint64_t  propprodd_ns = 0;

#endif

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
#ifdef ENABLE_RUNTIME_TIME
    auto t0 = Clock::now();
#endif
    TwoSumF(a, b, x, dx);
    *dx = *dx + da + db;
#ifdef ENABLE_RUNTIME_TIME
    auto t1 = Clock::now();
    propsumf_calls++;
    propsumf_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
}
void PropSumDError(double a, double da, double b, double db, double *x, double *dx) {
#ifdef ENABLE_RUNTIME_TIME
    auto t0 = Clock::now();
#endif
    TwoSumD(a, b, x, dx);
    *dx = *dx + da + db;

#ifdef ENABLE_RUNTIME_TIME
    auto t1 = Clock::now();
    propsumd_calls++;
    propsumd_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
}
void PropProdFError(float a, float da, float b, float db, float *x, float *dx) {
#ifdef ENABLE_RUNTIME_TIME
    auto t0 = Clock::now();
#endif
    TwoProdF(a, b, x, dx);
    *dx = *dx + a * db + b * da;
#ifdef ENABLE_RUNTIME_TIME
    auto t1 = Clock::now();
    propprodf_calls++;
    propprodf_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
}
void PropProdDError(double a, double da, double b, double db, double *x, double *dx) {
#ifdef ENABLE_RUNTIME_TIME
    auto t0 = Clock::now();
#endif
    TwoProdD(a, b, x, dx);
    *dx = *dx + a * db + b * da;
#ifdef ENABLE_RUNTIME_TIME
    auto t1 = Clock::now();
    propprodd_calls++;
    propprodd_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
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

extern "C" void report_fp_profile() {
#ifdef ENABLE_RUNTIME_TIME
    std::printf("\n[fp runtime profile]\n");
    std::printf("PropSumFError : calls=%llu total_ns=%llu avg_ns=%.2f\n",
        (unsigned long long)propsumf_calls,
        (unsigned long long)propsumf_ns,
        propsumf_calls ? (double) propsumf_ns / propsumf_calls : 0.0);

    std::printf("PropSumDError : calls=%llu total_ns=%llu avg_ns=%.2f\n",
        (unsigned long long)propsumd_calls,
        (unsigned long long)propsumd_ns,
        propsumd_calls ? (double) propsumd_ns / propsumd_calls : 0.0);

    std::printf("PropProdFError: calls=%llu total_ns=%llu avg_ns=%.2f\n",
        (unsigned long long)propprodf_calls,
        (unsigned long long)propprodf_ns,
        propprodf_calls ? (double) propprodf_ns / propprodf_calls : 0.0);

    std::printf("PropProdDError: calls=%llu total_ns=%llu avg_ns=%.2f\n",
        (unsigned long long)propprodd_calls,
        (unsigned long long)propprodd_ns,
        propprodd_calls ? (double) propprodd_ns / propprodd_calls : 0.0);
#endif
}