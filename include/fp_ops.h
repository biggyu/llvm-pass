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
    Branch = 15, ConvSI = 16, ConvUI = 17, Unknown = 18,
};

enum class ErrKind : uint8_t {
    Sensitivity, Cancellation,
};

struct SplitGamma {
    double value;
    ErrKind kind;
    bool exact;
};

#ifdef __cplusplus
}
#endif