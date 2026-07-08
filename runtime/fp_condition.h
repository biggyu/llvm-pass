#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
enum class FpOp : uint32_t {
    Add = 0, Sub = 1, Mul = 2, Div = 3,
    Sqrt = 4, Cbrt = 5,
    Log = 6, Exp = 7, Pow = 8,
    Sin = 9, Cos = 10, Tan = 11,
    Acos = 12, Asin = 13, Atan = 14,
    Unknown = 15,
};

enum class ErrKind : uint8_t {
    Sensitivity, Cancellation,
};

struct SplitGamma {
    double value;
    ErrKind kind;
    bool exact;
};

void condition_number_double(uint32_t opraw, double a, double da, double b, double db, bool aExact, bool bExact, uint32_t siteId);
void condition_number_float(uint32_t opraw, float a, float da, float b, float db, bool aExact, bool bExact, uint32_t siteId);

#ifdef __cplusplus
}
#endif