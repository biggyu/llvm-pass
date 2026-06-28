#pragma once
#include "../runtime/shadow_table.h"
#ifdef __cplusplus
extern "C" {
#endif

void shadow_store_double(void* addr, double x, double rhat, double dx, bool sign, bool isExact, double ehat);
void shadow_store_float(void* addr, float x, double rhat, double dx, bool sign, bool isExact, double ehat);
ShadowEntry* shadow_load(void* addr);
// void shadow_store_double(void* addr, double x, double rhat, double dx, bool sign, bool isExact, double ehat);
// ShadowEntry* shadow_load_double(void* addr);
// void shadow_store_float(void* addr, double x, double rhat, double dx, bool sign, bool isExact, double ehat);
// ShadowEntry* shadow_load_float(void* addr);


void report_smem_profile();

#ifdef __cplusplus
}
#endif