#include "smem_runtime.h"
#include "shadow_table.h"
#include <iostream>
#ifdef ENABLE_PROFILE
#include <chrono>
using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

struct OpProf {
    uint64_t calls = 0, ns = 0;
};
static OpProf shadowstore, shadowload, shadowpush, shadowpop;

struct ProfScope {
    OpProf &p;
    Clock::time_point t0;
    ProfScope(OpProf &p_) : p(p_), t0(Clock::now()) {}
    ~ProfScope() {
        p.ns += std::chrono::duration_cast<ns>(Clock::now() - t0).count();
        p.calls++;
    }
};
    #define PROFILE(slot) ProfScope _ps(slot)
#else
    #define PROFILE(slot) ((void)0)
#endif

static ShadowTable *s_tbl = nullptr;
static ShadowStack *s_stk = nullptr;

static ShadowTable& getTbl() {
    if (!s_tbl) {
        s_tbl = new ShadowTable();
    }
    return *s_tbl;
}

static ShadowStack& getStk() {
    if (!s_stk) {
        s_stk = new ShadowStack();
    }
    return *s_stk;
}

extern "C" void shadow_store_double(void* addr, double xhat, double rhat, double fp_val, double relerr) {
    PROFILE(shadowstore);
    getTbl().insert(addr, xhat, rhat, fp_val, relerr);
}
extern "C" void shadow_store_float(void* addr, float xhat, double rhat, float fp_val, double relerr) {
    PROFILE(shadowstore);
    getTbl().insert(addr, (double)xhat, rhat, (double)fp_val, relerr);
} 

extern "C" void shadow_load_double(void* addr, double progVal, ShadowEntry* out) {
    PROFILE(shadowload);
    ShadowEntry *e = getTbl().get(addr, progVal);
    if (e) {
        out = e;
    }
    getTbl().insert(addr, progVal, 0.0, progVal, 0.0);
    out = getTbl().get(addr, progVal);
}
extern "C" void shadow_load_float(void* addr, float progVal, ShadowEntry* out) {
    PROFILE(shadowload);
    ShadowEntry *e = getTbl().get(addr, (double)progVal);
    if (e) {
        out = e;
    }
    getTbl().insert(addr, progVal, 0.0, (double)progVal, 0.0);
    out = getTbl().get(addr, (double)progVal);
}

extern "C" void shadow_stack_push(double xhat, double rhat, double fp_val, double relerr) {
    PROFILE(shadowpush);
    getStk().push(xhat, rhat, fp_val, relerr);
}

extern "C" void shadow_stack_pop(ShadowEntry* out) {
    PROFILE(shadowpop);
    ShadowEntry *e = getStk().pop();
    out = e? e : new ShadowEntry{(uintptr_t)0.0, 0.0, 0.0, 0.0, 0.0};
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