#include "fp_debug.h"

#include <cstdio>
#include <cmath>
#include <cstdint>
#include <string>
// #include <vector>
// #include <cstring>
// #include <algorithm>
// #include <unordered_map>
// static uint64_t total_checks_double = 0;
// static uint64_t total_checks_float = 0;

enum class ErrorClass : uint8_t {
    Exact, Normal, TotalLoss,
    NaN, Inf, XZero
};

struct GlobalStats {
    uint64_t above_thres = 0;
    uint64_t nan = 0;
    uint64_t inf = 0;
    uint64_t branch_flips = 0;
    uint64_t cancellation = 0;

    uint64_t exact = 0;
    uint64_t normal = 0;
    uint64_t total_loss = 0;
    uint64_t total_checks = 0;
};
static GlobalStats G;

// struct SiteStats {
//     uint64_t cnt = 0;
//     uint64_t finite_cnt = 0;

//     double sum_bits = 0.0;
//     double max_bits = 0.0;

//     double sum_err = 0.0;
//     double max_relerr = 0.0;
//     double max_abs_dx = 0.0;

//     uint64_t warn_4 = 0;
//     uint64_t warn_8 = 0;
//     uint64_t warn_16 = 0;
//     uint64_t warn_prec = 0;

//     uint64_t exact = 0;
//     uint64_t normal = 0;
//     uint64_t total_loss = 0;
//     uint64_t inf = 0;
//     uint64_t nan = 0;
//     uint64_t xzero = 0;

//     double sample_x = 0.0;
//     double sample_dx = 0.0;

//     double sample_xzero_x = 0.0;
//     double sample_xzero_dx = 0.0;
// };

// struct SiteInfo {
//     std::string function;
//     std::string file;
//     int line = 0;
//     int col = 0;
//     std::string opcode;
// };

// static std::unordered_map<int, SiteStats> double_sites;
// static std::unordered_map<int, SiteStats> float_sites;
// static std::unordered_map<int, SiteInfo> site_infos;

enum FCmpPred {
    FCMP_FALSE = 0,
    FCMP_OEQ = 1,
    FCMP_OGT = 2,
    FCMP_OGE = 3,
    FCMP_OLT = 4,
    FCMP_OLE = 5,
    FCMP_ONE = 6,
    FCMP_ORD = 7,
    FCMP_UNO = 8,
    FCMP_UEQ = 9,
    FCMP_UGT = 10,
    FCMP_UGE = 11,
    FCMP_ULT = 12,
    FCMP_ULE = 13,
    FCMP_UNE = 14,
    FCMP_TRUE = 15,
};

static int CANCELLATION_THRESHOLD = 2;
// static int CANCELLATION_THRESHOLD = DEFAULT_CANCELLATION_THRESHOLD;
static void load_threshold() {
    if (const char *env = std::getenv("CANCELLATION_THRESHOLD")) {
        CANCELLATION_THRESHOLD = std::atoi(env);
    }
}

template <typename T>
static ErrorClass classify(T x, T dx) {
    if (std::isnan(x) || std::isnan(dx)) {
        return ErrorClass::NaN;
    }
    if (std::isinf(x) || std::isinf(dx)) {
        return ErrorClass::Inf;
    }
    if (dx == T(0)) {
        return ErrorClass::Exact;
    }
    if (x == T(0)) {
        return ErrorClass::XZero;
    }
    if (std::fabs(dx) >= std::fabs(x)) {
        return ErrorClass::TotalLoss;
    }
    return ErrorClass::Normal;
}

// extern "C" void register_fp_site(int site_id, const char* function, const char *file, int line, int col, const char* opcode) {
//     if (site_infos.find(site_id) != site_infos.end()) {
//         return;
//     }
//     SiteInfo info;
//     info.function = function ? function : "<unknown>";
//     info.file = file ? file : "<unknown>";
//     info.line = line;
//     info.col = col;
//     info.opcode = opcode ? opcode : "<unknown>";
//     site_infos[site_id] = std::move(info);
// }

template <typename T>
static double relative_error(T x, T dx) {
    if (!std::isfinite(x) || !std::isfinite(dx)) {
        return std::numeric_limits<double>::infinity();
    }
    if (dx == T(0)) {
        return 0.0;
    }
    if (x == T(0)) {
        return std::numeric_limits<double>::infinity();
    }
    return std::fabs(static_cast<double>(dx)) / std::fabs(static_cast<double>(x));
}

template <typename T>
static constexpr double precision_bits() {
    return std::is_same_v<T, float> ? 24.0 : 53.0;
}

template <typename T>
double incorrect_bits(T x, T dx, int metric) {
    constexpr int p = std::is_same_v<T, float> ? 24 : 53;
    double relerr = relative_error<T>(x, dx);

    if (relerr == 0.0) {
        return 0.0;
    }
    if (!std::isfinite(relerr)) {
        return std::numeric_limits<double>::infinity();
    }
    double bits = static_cast<double>(p) + std::log2(relerr);
    return bits > 0.0 ? bits : 0.0;
}

template <typename T>
double incorrect_bits_bitwise(T x, double dx) {
    using Bits = std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t>;
    constexpr int total = std::is_same_v<T, float> ? 24 : 53;
    if (dx == 0.0) {
        return 0.0;
    }
    double corrected_d = static_cast<double>(x) + static_cast<double>(dx);
    T corrected = static_cast<T>(corrected_d);
    if (x == corrected) {
        return 0.0;
    }
    Bits bx, bc, diff;
    std::memcpy(&bx, &x, sizeof(T));
    std::memcpy(&bc, &corrected, sizeof(T));
    diff = bx ^ bc;
    if (diff == 0.0) {
        return 0.0;
    }
    int hi = (total - 1) - __builtin_clzll((unsigned long long) diff << (64 - total));
    return static_cast<double>(hi + 1);
}

// void check_cancellation(double a, double b, double result) {
//     if (result == 0.0 || a == 0.0 || b == 0.0) {
//         return;
//     }
//     int e_max = std::max(std::ilogb(a), std::ilogb(b));
//     int e_res = std::ilogb(result);
//     int cancelled = e_max - e_res;
//     if (cancelled >= CANCELLATION_THRESHOLD) {
//         G.cancellation++;
//     }
// }

bool eval_pred(double a, double b, int pred) {
    bool nan = std::isnan(a) || std::isnan(b);
    switch (pred) {
        case FCMP_FALSE: return false;
        
        case FCMP_OEQ:  return !nan && (a == b);
        case FCMP_OGT:  return !nan && (a > b);
        case FCMP_OGE:  return !nan && (a >= b);
        case FCMP_OLT:  return !nan && (a < b);
        case FCMP_OLE:  return !nan && (a <= b);
        case FCMP_ONE:  return !nan && (a != b);
        case FCMP_ORD:  return !nan;

        case FCMP_UNO:  return nan;
        case FCMP_UEQ:  return nan || (a == b);
        case FCMP_UGT:  return nan || (a > b);
        case FCMP_UGE:  return nan || (a >= b);
        case FCMP_ULT:  return nan || (a < b);
        case FCMP_ULE:  return nan || (a <= b);
        case FCMP_UNE:  return nan || (a != b);
        case FCMP_TRUE: return true;
        default:        return false;
    }
}
void check_branch(double a, double da, double b, double db, int pred) {
    double a_corr = a + da;
    double b_corr = b + db;

    bool corrected = eval_pred(a_corr, b_corr, pred);
    bool prog = eval_pred(a, b, pred);
    if(corrected != prog) {
        G.branch_flips++;
    }
}

template <typename T>
static void check_error_impl(T x, double dx, int site_id, int metric) {
// static void check_error_impl(T x, double dx, int site_id, int metric, uint64_t &total_checks, std::unordered_map<int, SiteStats> &sites) {
    ErrorClass errcls = classify<T>(x, dx);
    double bits = incorrect_bits_bitwise(x, dx);
    // double bits = incorrect_bits<T>(x, dx, metric);
    // double relerr = relative_error<T>(x, dx);
    // double precision = precision_bits<T>();

    G.total_checks++;

    SiteStats &S = sites[site_id];
    S.cnt++;
    switch (errcls) {
        case ErrorClass::Exact:
            S.exact++;
            break;
        case ErrorClass::Normal:
            S.normal++;
            break;
        case ErrorClass::TotalLoss:
            S.total_loss++;
            break;
        case ErrorClass::Inf:
            S.inf++;
            return;
        case ErrorClass::NaN:
            S.nan++;
            return;
        case ErrorClass::XZero:
            S.xzero++;
            return;
    }
    if (bits > 45.0) {
        G.above_thres++;
    }
}

extern "C" void check_error_double(double x, double dx, int site_id, int metric) {
    return check_error_impl<double>(x, dx, site_id, metric);
}

extern "C" void check_error_float(float x, double dx, int site_id, int metric) {
    return check_error_impl<float>(x, dx, site_id, metric);
}

// template <typename T>
// static void report_top_impl(std::unordered_map<int, SiteStats> &site_map) {
//     std::vector<std::pair<int, SiteStats>> sites;

//     for (const auto &KV : site_map) {
//         sites.push_back(KV);
//     }

//     std::sort(sites.begin(), sites.end(),
//         [](const auto &A, const auto &B) {
//             const SiteStats &SA = A.second;
//             const SiteStats &SB = B.second;

//             return SA.max_bits > SB.max_bits;
//         }
//     );

//     size_t limit = std::min<size_t>(10, sites.size());

//     printf("\nTop %s sites by numerical severity::\n", precision_bits<T>() == 24.0 ? "float" : "double");

//     for (size_t i = 0; i < limit; i++) {
//         int site_id = sites[i].first;
        
//         auto It = site_infos.find(site_id);
//         if (It != site_infos.end()) {
//             const SiteInfo &Info = It->second;

//             printf("[%zu] site=%d %s:%d:%d function=%s opcode=%s\n",
//                 i + 1,
//                 site_id,
//                 Info.file.c_str(),
//                 Info.line,
//                 Info.col,
//                 Info.function.c_str(),
//                 Info.opcode.c_str());
//         } else {
//             printf("[%zu] site=%d <no source info>\n", i + 1, site_id);
//         }

//         const SiteStats &S = sites[i].second;

//         double avg_bits = S.cnt ? S.sum_bits / S.finite_cnt : 0.0;
//         double avg_relerr = S.cnt ? S.sum_err / S.finite_cnt : 0.0;

//         printf("    site=%d count=%llu finite count=%llu max_bits=%.2f avg_bits=%.2f "
//                "max_relerr=%.3e avg_relerr=%.3e max_abs_dx=%.3e\n",
//                site_id,
//                (unsigned long long)S.cnt,
//                (unsigned long long)S.finite_cnt,
//                S.max_bits,
//                avg_bits,
//                S.max_relerr,
//                avg_relerr,
//                S.max_abs_dx);

//         printf("    warnings: >4=%llu >8=%llu >16=%llu >precision=%llu\n",
//                (unsigned long long)S.warn_4,
//                (unsigned long long)S.warn_8,
//                (unsigned long long)S.warn_16,
//                (unsigned long long)S.warn_prec);

//         printf("    classes: exact=%llu normal=%llu total_loss=%llu inf=%llu nan=%llu xzero=%llu\n",
//                (unsigned long long)S.exact,
//                (unsigned long long)S.normal,
//                (unsigned long long)S.total_loss,
//                (unsigned long long)S.inf,
//                (unsigned long long)S.nan,
//                (unsigned long long)S.xzero);

//         printf("    sample: x=%.17e dx=%.17e\n",
//                S.sample_x,
//                S.sample_dx);
//     }
// }

// extern "C" void report_top_double(std::unordered_map<int, SiteStats> &site_map) {
//     return report_top_impl<double>(site_map);
// }

// extern "C" void report_top_float(std::unordered_map<int, SiteStats> &site_map) {
//     return report_top_impl<float>(site_map);
// }

extern "C" void report_debug_summary() {
    printf("Error above 50 bits found %llu\n", (unsigned long long) G.above_thres);
    printf("Total NaN found %llu\n", (unsigned long long) G.nan);
    printf("Total Inf Found %llu\n", (unsigned long long) G.inf);
    printf("Total branch flips found %llu\n", (unsigned long long) G.branch_flips);
    printf("Total catastrophic cancellation found %llu\n\n", (unsigned long long) G.cancellation);
}