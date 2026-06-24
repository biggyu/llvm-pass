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
static shadowstore, shadowload;

struct ProfScope {
    OpProf &p;
    Clock::time_point t0;
    ProfScope(OpProf &p_) : p(p_), t0(Clock::now()) {}
    ~ProfScopt() {
        p.ns += std::chorono::duration_cast<ns>(Clock::now() - t0).count();
        p.calls++;
    }
}
    #define PROFILE(slot) ProfScopt _ps(slot)
#else
    #define PROFILE(slot) ((void)0)
#endif

static ShadowTable s_tbl;
extern "C" void shadow_store(void* addr, double x, double rhat, double dx, bool sign, bool isExact, double ehat) {
    PROFILE(shadowstore);
    s_tbl.insert(addr, x, rhat, dx, sign, isExact, ehat);
} 
extern "C" ShadowEntry* shadow_load(void* addr) {
    PROFILE(shadowloadd);
    return s_tbl.get(addr);
}
// extern "C" void shadow_store_float(void* addr, double x, double rhat, double dx, bool sign, bool isExact, double ehat) {
//     PROFILE(storef);
//     s_tbl.insert(addr, x, rhat, dx, sign, isExact, ehat);
// } 
// extern "C" ShadowEntry* shadow_load_float(void* addr) {
//     PROFILE(loadf);
//     return s_tbl.get(addr);
// }

extern "C" void report_smem_profile() {
#ifdef ENABLE_PROFILE
    std::printf("\n[smem runtime profile]\n");
    std::printf("shadow_store_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowstore.calls, (unsigned long long)shadowstore.ns, 
    shadowstore.calls ? (double)shadowstore.ns / shadowstore.calls : 0.0);
    std::printf("shadow_load_double: calls=%llu total_ns=%llu avg_ns=%.2f\n", (unsigned long long)shadowload.calls, (unsigned long long)shadowload.ns, 
    shadowload.calls ? (double)shadowload.ns / shadowload.calls : 0.0);
#endif
}