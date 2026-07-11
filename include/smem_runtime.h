#pragma once
#include "shadow_table.h"
#ifdef __cplusplus
extern "C" {
#endif

// void shadow_store_double(void* addr, double x, double dx);
// void shadow_store_float(void* addr, float x, double dx);


void shadow_store_double(void* addr, double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr);
void shadow_store_float(void* addr, float xhat, double rhat, bool sign, double ehat, bool isExact, double relerr);
ShadowEntry* shadow_load_double(void* addr, double progVal);
ShadowEntry* shadow_load_float(void* addr, float progVal);

void shadow_stack_push(double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr);
ShadowEntry* shadow_stack_pop();


void report_smem_profile();

#ifdef __cplusplus
}
#endif