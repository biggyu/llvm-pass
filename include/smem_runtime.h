#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void shadow_store_double(void* addr, double x, double dx);
double shadow_load_double(void* addr, double progVal);
void shadow_store_float(void* addr, float x, double dx);
double shadow_load_float(void* addr, float progVal);

void shadow_stack_push(double dx);
double shadow_stack_pop();

void report_smem_profile();

#ifdef __cplusplus
}
#endif