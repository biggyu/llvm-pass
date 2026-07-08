#include "smem_runtime.h"
#include "shadow_table.h"
// #include <unordered_map>
// #include <cstdint>
#include <iostream>
#ifdef ENABLE_PROFILE
#include <chrono>
using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

struct OpProf {
    uint64_t calls = 0, ns = 0;
};
static shadowstore, shadowload, shadowpush, shadowpop

struct ProfScope {
    OpProf &p;
    Clock::time_point t0;
    ProfScope(OpProf &p_) : p(p_), t0(Clock::now()) {}
    ~ProfScope() {
        p.ns += std::chrono::duration_cast<ns>(Clock::now() - t0).count();
        p.calls++;
    }
}
    #define PROFILE(slot) ProfScope _ps(slot)
#else
    #define PROFILE(slot) ((void)0)
#endif

static ShadowTable s_tbl;
extern "C" void shadow_store_double(void* addr, double x, double dx) {
    PROFILE(shadowstore);
    s_tbl.insert(addr, x, dx);
} 
extern "C" double shadow_load_double(void* addr, double progVal) {
    PROFILE(shadowload);
    return s_tbl.get(addr, progVal);
}
extern "C" void shadow_store_float(void* addr, float x, double dx) {
    PROFILE(shadowstore);
    s_tbl.insert(addr, (double)x, dx);
} 
extern "C" double shadow_load_float(void* addr, float progVal) {
    PROFILE(shadowload);
    return s_tbl.get(addr, (double)progVal);
}

static ShadowStack s_stk;
extern "C" void shadow_stack_push(double err) {
    PROFILE(shadowpush);
    s_stk.push(err);
}

extern "C" double shadow_stack_pop() {
    PROFILE(shadowpop);
    return s_stk.pop();
}

extern "C" void report_smem_profile() {
#ifdef ENABLE_PROFILE
    std::printf("\n[smem runtime profile]\n");
    std::printf("shadow_store_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowstore.calls, (unsigned long long)shadowstore.ns, 
    shadowstore.calls ? (double)shadowstore.ns / shadowstore.calls : 0.0);
    std::printf("shadow_load_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowload.calls, (unsigned long long)shadowload.ns, 
    shadowload.calls ? (double)shadowload.ns / shadowload.calls : 0.0);
#endif
}