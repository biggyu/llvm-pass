#pragma once
#include "shadow_table.h"
#ifdef __cplusplus
extern "C" {
#endif

void shadow_store_double(void* addr, double xhat, double rhat, double fp_val, double relerr);
void shadow_store_float(void* addr, float xhat, double rhat, float fp_val, double relerr);
void shadow_load_double(void* addr, double progVal, ShadowEntry* out);
void shadow_load_float(void* addr, float progVal, ShadowEntry* out);

void shadow_stack_push(double xhat, double rhat, double fp_val, double relerr);
void shadow_stack_pop(ShadowEntry* out);


void report_smem_profile();

#ifdef __cplusplus
}
#endif