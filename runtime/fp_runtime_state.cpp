#include "fp_runtime_state.h"

GlobalStats G;
std::unordered_map<uint32_t, SiteInfo>& site_infos() {
    static std::unordered_map<uint32_t, SiteInfo> m;
    return m;
}
std::unordered_map<uint32_t, SiteStats>& site_stats() {
    static std::unordered_map<uint32_t, SiteStats> m;
    return m;
}