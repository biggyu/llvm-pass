#include "smem_runtime.h"
// #include "shadow_table.h"
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

extern "C" void shadow_store_double(void* addr, double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr) {
    PROFILE(shadowstore);
    s_tbl.insert(addr, xhat, rhat, sign, ehat, isExact, relerr);
}
extern "C" void shadow_store_float(void* addr, float xhat, double rhat, bool sign, double ehat, bool isExact, double relerr) {
    PROFILE(shadowstore);
    s_tbl.insert(addr, (double)xhat, rhat, sign, ehat, isExact, relerr);
} 

extern "C" ShadowEntry shadow_load_double(void* addr, double progVal) {
    PROFILE(shadowload);
    ShadowEntry *e = s_tbl.get(addr, progVal);
    if (e) {
        return *e;
    }
    s_tbl.insert(addr, progVal, 0.0, false, 0.0, true, 0.0);
    return *s_tbl.get(addr, progVal);
}
extern "C" ShadowEntry shadow_load_float(void* addr, float progVal) {
    PROFILE(shadowload);
    ShadowEntry *e = s_tbl.get(addr, (double)progVal);
    if (e) {
        return *e;
    }
    s_tbl.insert(addr, progVal, 0.0, false, 0.0, true, 0.0);
    return *s_tbl.get(addr, (double)progVal);
}

static ShadowStack s_stk;
extern "C" void shadow_stack_push(double xhat, double rhat, bool sign, double ehat, bool isExact, double relerr) {
    PROFILE(shadowpush);
    s_stk.push(xhat, rhat, sign, ehat, isExact, relerr);
}

extern "C" ShadowEntry shadow_stack_pop() {
    PROFILE(shadowpop);
    ShadowEntry *e = s_stk.pop();
    if (e) {
        return *e;
    }
    ShadowEntry z{};
    z.xhat = 0.0;
    z.rhat = 0.0;
    z.sign = false;
    z.ehat = 0.0;
    z.isExact = true;
    z.relerr = 0.0;
    return z;
}

extern "C" void report_smem_profile() {
#ifdef ENABLE_PROFILE
    std::printf("\n[smem runtime profile]\n");
    std::printf("shadow_store_double: calls=%llu total_ns=%l lu avg_ns=%.2f\n", (unsigned long long)shadowstore.calls, (unsigned long long)shadowstore.ns, 
    shadowstore.calls ? (double)shadowstore.ns / shadowstore.calls : 0.0);
    std::printf("shadow_load_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowload.calls, (unsigned long long)shadowload.ns, 
    shadowload.calls ? (double)shadowload.ns / shadowload.calls : 0.0);
#endif
}