#include "fp_debug.h"

#include <cstdio>
#include <cmath>
#include <cstdint>

#ifdef ENABLE_FP_DEBUG
static uint64_t total_checks_double = 0;
static uint64_t total_checks_float = 0;

static uint64_t warn_4_bits_double = 0;
static uint64_t warn_8_bits_double = 0;
static uint64_t warn_16_bits_double = 0;
static uint64_t warn_prec_bits_double = 0;

static uint64_t warn_4_bits_float = 0;
static uint64_t warn_8_bits_float = 0;
static uint64_t warn_16_bits_float = 0;
static uint64_t warn_prec_bits_float = 0;

static uint64_t printed_warnings = 0;
static constexpr uint64_t max_printed_warnings = 20;

static double max_bits_double = 0.0;
static int max_bits_double_site = -1;

static float max_bits_float = 0.0;
static int max_bits_float_site = -1;
#endif

template <typename T>
static ErrorClass classify(T x, T dx) {
    if (!std::isfinite(x) || !std::isfinite(dx)) {
        return ErrorClass::NaNOrInf;
    }
    if (dx == T(0)) {
        return ErrorClass::Exact;
    }
    if (x == T(0)) {
        return ErrorClass::XZero;
    }
    if (std::fabs(dx) >= std::fabs(x)) {
        return ErrorClass::TotalLoss;
    }
    return ErrorClass::Normal;
}

template <typename T>
double incorrect_bits(T x, T dx, int metric) {
    constexpr int p = std::is_same_v<T, float> ? 24 : 53;

    if (!std::isfinite(x) || !std::isfinite(dx)) {
        return std::numeric_limits<double>::infinity();
    }

    if (dx == T(0)) {
        return 0.0;
    }

    if (x == T(0)) {
        return std::numeric_limits<double>::infinity();
    }

    double relerr = 0.0;

    switch (metric) {
        case 0:
        default:
            relerr = std::fabs((double)dx) / std::fabs((double)x);
            break;
    }

    if (relerr == 0.0) {
        return 0.0;
    }

    if (!std::isfinite(relerr)) {
        return std::numeric_limits<double>::infinity();
    }

    double bits = p + std::log2(relerr);
    return bits > 0.0 ? bits : 0.0;
}

extern "C" void check_error_double(double x, double dx, int site_id, int metric) {
#ifdef ENABLE_FP_DEBUG
    ErrorClass errcls = classify<double>(x, dx);
    double bits = incorrect_bits<double>(x, dx, metric);
    // std::printf("%d %f\n", static_cast<int>(errcls), bits);
    // #if FP_DEBUG_METRICS == 0
    //     bits = incorrect_bits_relative<double>(x, dx);
    // #elif FP_DEBUG_METRIX == 1
    //     bits = incorrect_bits_ulp<double>(x, dx);
    // #else
    //     bits = incorrect_bits_hybrid<double>(x, dx);
    // #endif

    total_checks_double++;

    if (bits > max_bits_double) {
        max_bits_double = bits;
        max_bits_double_site = site_id;
    }
    if (bits >= 4.0) {
        warn_4_bits_double++;
    }
    if (bits >= 8.0) {
        warn_8_bits_double++;
    }
    if (bits >= 16.0) {
        warn_16_bits_double++;
    }
    if (bits >= 53.0) {
        warn_prec_bits_double++;
    }
    if (bits >= 16.0 && printed_warnings < max_printed_warnings) {
        std::printf("[fp-debug] double site %d x=%.17e dx=%.17e incorrect bits=%.2f\n",
            site_id, x, dx, bits
        );
        printed_warnings++;
    }
#endif
}

extern "C" void check_error_float(float x, float dx, int site_id, int metric) {
#ifdef ENABLE_FP_DEBUG
    ErrorClass errcls = classify<float>(x, dx);
    double bits = incorrect_bits<float>(x, dx, metric);
    // std::printf("%d %f", errcls, bits);
    // #if FP_DEBUG_METRIC == 0
    //     bits = incorrect_bits_relative<float>(x, dx);
    // #elif FP_DEBUG_METRIX == 1
    //     bits = incorrect_bits_ulp<float>(x, dx);
    // #else
    //     bits = incorrect_bits_hybrid<float>(x, dx);
    // #endif

    total_checks_float++;

    if (bits > max_bits_float) {
        max_bits_float = bits;
        max_bits_float_site = site_id;
    }
    if (bits >= 4.0) {
        warn_4_bits_float++;
    }
    if (bits >= 8.0) {
        warn_8_bits_float++;
    }
    if (bits >= 16.0) {
        warn_16_bits_float++;
    }
    if (bits >= 53.0) {
        warn_prec_bits_float++;
    }
    if (bits >= 16.0 && printed_warnings < max_printed_warnings) {
        std::printf("[fp-debug] float site %d x=%.17e dx=%.17e incorrect bits=%.2f\n",
            site_id, x, dx, bits
        );
        printed_warnings++;
    }
#endif
}

extern "C" void report_debug_summary() {
#ifdef ENABLE_FP_DEBUG
    printf("--- [fp debug summary] ---\n");

    printf("double checks=%llu\n", (unsigned long long)total_checks_double);
    printf("double warnings= >4: %llu, >8: %llu, >16: %llu, >53: %llu\n",
        (unsigned long long)warn_4_bits_double,
        (unsigned long long)warn_8_bits_double,
        (unsigned long long)warn_16_bits_double,
        (unsigned long long)warn_prec_bits_double
        );
    printf("double max bits=%.2f site=%d\n", max_bits_double, max_bits_double_site);

    printf("float checks=%llu\n", (unsigned long long)total_checks_float);
    printf("float warnings= >4: %llu, >8: %llu, >16: %llu, >53: %llu\n",
        (unsigned long long)warn_4_bits_float,
        (unsigned long long)warn_8_bits_float,
        (unsigned long long)warn_16_bits_float,
        (unsigned long long)warn_prec_bits_float
        );
    printf(" float max bits=%.2f site=%d\n", max_bits_float, max_bits_float_site);
#endif
}