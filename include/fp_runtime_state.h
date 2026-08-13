#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include "fp_ops.h"

struct GlobalStats {
    uint64_t above_thres = 0;
    uint64_t nan = 0;
    uint64_t inf = 0;
    uint64_t branch_flips = 0;
    uint64_t conv_cnt = 0;

    uint64_t exact = 0;
    uint64_t normal = 0;
    uint64_t xzero = 0;
    uint64_t total_loss = 0;
    uint64_t total_checks = 0;

    uint64_t cond_detected = 0;
    uint64_t cond_cancellation = 0;
    uint64_t cond_sensitivity = 0;
    uint64_t cond_untyped = 0;
    uint64_t cond_suppressed = 0;
};

struct SiteInfo {
    std::string function, file, opcode;
    int line = 0, col = 0;
};

struct SiteStats {
    uint64_t thres_hits = 0;
    uint64_t nan_hits = 0;
    uint64_t inf_hits = 0;

    uint64_t branch_hits = 0;
    uint64_t conv_hits = 0;

    uint64_t cancellation_hits = 0;
    uint64_t sensitivity_hits = 0;
    uint64_t suppressed_hits = 0;
    
    double max_bits = 0.0;
    double max_gamma = 0.0;
    double sample_operand = 0.0;
    
    ErrKind worst_kind;
};

extern GlobalStats G;
extern std::unordered_map<uint32_t, SiteInfo>& site_infos();
extern std::unordered_map<uint32_t, SiteStats>& site_stats();