#include "smem_runtime.h"
#include "shadow_table.h"
// #include <unordered_map>
// #include <cstdint>
// #include <iostream>

static ShadowTable s_tbl;
extern "C" void shadow_store_double(void* addr, double x, double dx) {
    s_tbl.insert(addr, x, dx);
} 
extern "C" double shadow_load_double(void* addr) {
    return s_tbl.getDouble(addr);
}
extern "C" void shadow_store_float(void* addr, float x, float dx) {
    s_tbl.insert(addr, x, dx);
} 
extern "C" float shadow_load_float(void* addr) {
    return s_tbl.getFloat(addr);
}
