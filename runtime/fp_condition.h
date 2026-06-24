#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
enum class FpOp : uint32_t {
    Add, Sub, Mul, Div,
    Sqrt, Cbrt,
    Log, Exp, Pow,
    Sin, Cos, Tan,
    Acos, Asin, Atan,
    Unknown,
};

enum class ErrKind : uint8_t {
    Sensitivity, Cancellation,
};

struct SplitGamma {
    double value;
    ErrKind kind;
    bool exact;
};

double condition_number_double(uint32_t opcode, double a, double da, double b, double db, bool aExact, bool bExact, uint32_t siteId);
double condition_number_float(uint32_t opcode, float a, float da, float b, float db, bool aExact, bool bExact, uint32_t siteId);

#ifdef __cplusplus
}
#endif