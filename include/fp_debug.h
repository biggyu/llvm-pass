#pragma once
#include <cstdint>

enum class ErrorClass : uint8_t {
    Exact,      // dx == 0
    Normal,     // 0 < |dx| < |x|
    TotalLoss,  // |dx| >= |x| — "more than precision bits incorrect"
    NaNOrInf,   // x or dx is non-finite
    XZero       // x == 0 but dx != 0 — special: any nonzero dx is total loss
};

#ifdef __cplusplus
extern "C" {
#endif

void check_error_double(double x, double dx, int site_id, int metric);
void check_error_float(float x, float dx, int site_id, int metric);

void report_debug_summary();

#ifdef __cplusplus
}
#endif