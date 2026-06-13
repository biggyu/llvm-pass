#include "smem_runtime.h"
#include "shadow_table.h"
// #include <unordered_map>
// #include <cstdint>
#include <iostream>
#ifdef ENABLE_PROFILE
#include <chrono>
using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

static uint64_t shadowstoref_calls = 0;
static uint64_t shadowstored_calls = 0;
static uint64_t shadowloadf_calls = 0;
static uint64_t shadowloadd_calls = 0;
static uint64_t shadowstoref_ns = 0;
static uint64_t shadowstored_ns = 0;
static uint64_t shadowloadf_ns = 0;
static uint64_t shadowloadd_ns = 0;

#endif

static ShadowTable s_tbl;
extern "C" void shadow_store_double(void* addr, double x, double dx) {
#ifdef ENABLE_PROFILE
    auto t0 = Clock::now();
#endif
    s_tbl.insert(addr, x, dx);
#ifdef ENABLE_PROFILE
    auto t1 = Clock::now();
    shadowstored_calls++;
    shadowstored_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
} 
extern "C" double shadow_load_double(void* addr) {
#ifdef ENABLE_PROFILE
    auto t0 = Clock::now();
#endif
    double result = s_tbl.getDouble(addr);
#ifdef ENABLE_PROFILE
    auto t1 = Clock::now();
    shadowloadd_calls++;
    shadowloadd_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
    return result;
}
extern "C" void shadow_store_float(void* addr, float x, float dx) {
#ifdef ENABLE_PROFILE
    auto t0 = Clock::now();
#endif
    s_tbl.insert(addr, x, dx);
#ifdef ENABLE_PROFILE
    auto t1 = Clock::now();
    shadowstoref_calls++;
    shadowstoref_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
} 
extern "C" float shadow_load_float(void* addr) {
#ifdef ENABLE_PROFILE
    auto t0 = Clock::now();
#endif
    float result = s_tbl.getFloat(addr);
#ifdef ENABLE_PROFILE
    auto t1 = Clock::now();
    shadowloadf_calls++;
    shadowloadf_ns += std::chrono::duration_cast<ns>(t1 - t0).count();
#endif
    return result;
}

extern "C" void report_smem_profile() {
#ifdef ENABLE_PROFILE
    std::printf("\n[smem runtime profile]\n");
    std::printf("shadow_store_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowstored_calls, (unsigned long long)shadowstored_ns, 
    shadowstored_calls ? (double)shadowstored_ns / shadowstored_calls : 0.0);
    std::printf("shadow_store_float: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowstoref_calls, (unsigned long long)shadowstoref_ns, 
    shadowstoref_calls ? (double)shadowstoref_ns / shadowstoref_calls : 0.0);
    std::printf("shadow_load_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowloadd_calls, (unsigned long long)shadowloadd_ns, 
    shadowloadd_calls ? (double)shadowloadd_ns / shadowloadd_calls : 0.0);
    std::printf("shadow_load_float: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowloadf_calls, (unsigned long long)shadowloadf_ns, 
    shadowloadf_calls ? (double)shadowloadf_ns / shadowloadf_calls : 0.0);
#endif
}