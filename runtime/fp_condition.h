#pragma once
#include <cstdint>
#include "fp_ops.h"
#ifdef __cplusplus
extern "C" {
#endif

double condition_number(uint32_t opraw, double a, double a_Ex, double b, double b_Ex, double aVal, double bVal, uint32_t siteId);

#ifdef __cplusplus
}
#endif