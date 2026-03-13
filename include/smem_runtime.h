#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void shadow_store(void* addr, double x, double dx);
double shadow_load(void* addr);

#ifdef __cplusplus
}
#endif