#include <cstdio>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>
#include <climits>
#include <algorithm>
#include <unordered_map>
#include "fp_debug.h"
#include "fp_condition.h"
#include "fp_ops.h"
#include "fp_runtime_state.h"
static double load_error_threshold() {
    if (const char *s = std::getenv("FPCHECK_BITS")) {
        char *end = nullptr;
        double v = std::strtod(s, &end);
        if (end != s) return v;
        std::fprintf(stderr, "[fpcheck] ignoring invalid FPCHECK_BITS \"%s\"\n", s);
    }
    return 50.0;
}
double g_error_threshold = load_error_threshold();

enum class ErrorClass : uint8_t {
    Exact, Normal, TotalLoss,
    NaN, Inf, XZero
};

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

static ErrorClass classify(double x, double dx) {
    if (std::isnan(x)) {
        return ErrorClass::NaN;
    }
    if (std::isinf(x)) {
        return ErrorClass::Inf;
    }
    if (dx == double(0)) {
        return ErrorClass::Exact;
    }
    if (x == double(0)) {
        return ErrorClass::XZero;
    }
    if (std::fabs(dx) >= std::fabs(x)) {
        return ErrorClass::TotalLoss;
    }
    return ErrorClass::Normal;
}

void register_fp_site(int site_id, const char* function, const char *file, int line, int col, const char* opcode) {
    if (site_infos().find(site_id) != site_infos().end()) {
        return;
    }
    SiteInfo info;
    info.function = function ? function : "<unknown>";
    info.file = file ? file : "<unknown>";
    info.line = line;
    info.col = col;
    info.opcode = opcode ? opcode : "<unknown>";
    site_infos()[site_id] = std::move(info);
    site_stats()[site_id] = {};
}

template <typename T>
static double relative_error(T x, double dx) {
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
double incorrect_bits_relerr(T x, double dx, int metric) {
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

unsigned long ulp_dist(double x, double y) {
    if (x == 0) {
        x = 0;
    }
    if (y == 0) {
        y = 0;
    }
    if (x != x) {
        return ULLONG_MAX - 1;
    }
    if (y != y) {
        return ULLONG_MAX - 1;
    }
    long long xx = *((long long *)&x);
    xx = xx < 0 ? LLONG_MIN - xx : xx;
    long long yy = *((long long *)&y);
    yy = yy < 0 ? LLONG_MIN - yy : yy;
    return xx >= yy ? xx - yy : yy - xx;
}

double incorrect_bits_ulp(double computed_val, double error) {
    double shadow_rounded = error + computed_val;   
    unsigned long ulp_err = ulp_dist(shadow_rounded, computed_val);
    return log2((double)ulp_err + 1.0);
}

void check_conv_si(int val, double src, double src_err, uint32_t site_id) {
    SiteStats &SS = site_stats()[site_id];
    double conv = src + src_err;
    if(!std::isfinite(conv)) {
        return;
    }
    int real_val = static_cast<int>(conv);
    if (real_val != val) {
        G.conv_cnt++;
        SS.conv_hits++;
    }
}

void check_conv_ui(size_t val, double src, double src_err, uint32_t site_id) {
    SiteStats &SS = site_stats()[site_id];
    double conv = src + src_err;
    if(!std::isfinite(conv)) {
        return;
    }
    size_t real_val = static_cast<size_t>(conv);
    if (real_val != val) {
        G.conv_cnt++;
        SS.conv_hits++;
    }
}

static bool eval_pred(double a, double b, size_t pred) {
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
extern "C" void check_branch(double a, double da, double b, double db, size_t pred, bool computed_res, uint32_t site_id) {
    SiteStats &SS = site_stats()[site_id];

    bool corrected = eval_pred(a + da, b + db, pred);
    if(corrected != computed_res) {
        G.branch_flips++;
        SS.branch_hits++;
    }
}

void check_error(double x, double dx, uint32_t site_id, int metric) {
    SiteStats &SS = site_stats()[site_id];
    ErrorClass errcls = classify(x, dx);
    double ulp = incorrect_bits_ulp(x, dx);
    // double bitwise = incorrect_bits_bitwise(x, dx);
    // double relerr = incorrect_bits_relerr<T>(x, dx, metric);
    // double relerr = relative_error<T>(x, dx);
    // double precision = precision_bits<T>();

    G.total_checks++;

    // SiteStats &S = sites[site_id];
    // S.cnt++;
    switch (errcls) {
        case ErrorClass::Exact:
            G.exact++;
            break;
        case ErrorClass::Normal:
            G.normal++;
            break;
        case ErrorClass::TotalLoss:
            G.total_loss++;
            break;
        case ErrorClass::Inf:
            G.inf++;
            SS.inf_hits++;
            return;
        case ErrorClass::NaN:
            G.nan++;
            SS.nan_hits++;
            return;
        case ErrorClass::XZero:
            G.xzero++;
            return;
    }
    if (ulp > SS.max_bits) {
        SS.max_bits = ulp;
    }
    if (ulp >= g_error_threshold) {
        G.above_thres++;
        SS.thres_hits++;
    }
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

//         printf("    classes: exact=%llu normal=%llu total_loss=%llu nan_inf=%llu xzero=%llu\n",
//                (unsigned long long)S.exact,
//                (unsigned long long)S.normal,
//                (unsigned long long)S.total_loss,
//                (unsigned long long)S.nan_or_inf,
//                (unsigned long long)S.xzero);

//         printf("    condition: max_gamma=%.3e kind=%s, cancellation_hits=%llu, sensitivity_hits=%llu, (operand=%.6e)\n",
//                 S.max_gamma,
//                 (unsigned long long)S.cond_cancellation,
//                 (unsigned long long)S.cond_sensitivity,
//                 S.sample_gamma_operand);

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
    const char *dir = getenv("ERRLOG_DIR");
    char path[1024];
    if (dir && dir[0] != '\0') {
        snprintf(path, sizeof(path), "%s/error.log", dir);
    } else {
        snprintf(path, sizeof(path), "examples/error.log");
    }
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "Error above bits %d found %llu\n", (int)g_error_threshold, (unsigned long long) G.above_thres);
    fprintf(f, "Total NaN found %llu\n", (unsigned long long) G.nan);
    fprintf(f, "Total Inf found %llu\n", (unsigned long long) G.inf);
    fprintf(f, "Total branch flips found %llu\n", (unsigned long long) G.branch_flips);
    fprintf(f, "Total conversion errors found %llu\n\n", (unsigned long long) G.conv_cnt);

    fprintf(f, "Condition-number detections found %llu\n", (unsigned long long)G.cond_detected);
    fprintf(f, "Total cancellation found %llu\n", (unsigned long long)G.cond_cancellation);
    fprintf(f, "Total sensitivity found %llu\n", (unsigned long long)G.cond_sensitivity);
    fprintf(f, "Total untyped found %llu\n", (unsigned long long)G.cond_untyped);
    fprintf(f, "Total suppressed found %llu\n\n", (unsigned long long)G.cond_suppressed);

    struct Row {
        uint32_t id;
        const SiteStats *ss;
    };
    std::vector<Row> rows;
    for (auto &kv : site_stats()) {
        SiteStats &s = kv.second;
        uint64_t total = s.thres_hits + s.nan_hits + s.inf_hits + 
                    s.branch_hits + s.conv_hits +
                    s.cancellation_hits + s.sensitivity_hits + s.suppressed_hits;
        if (total > 0) {
            rows.push_back({kv.first, &s});
        }
    }
    
    if (!rows.empty()) {
        std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
            if (a.ss->max_bits != b.ss->max_bits) {
                return a.ss->max_bits > b.ss->max_bits;
            }
            return a.ss->max_gamma > b.ss->max_gamma;
        });
    }

    fprintf(f, "--- Top Error Sites ---\n");
    int printed = 0;
    for (auto &kv : rows) {
        if(printed++ >= 10) {
            break;
        }
        auto it = site_infos().find(kv.id);
        std::string file = "<unknown>", op = "?";
        int line = 0, col = 0;
        if (it != site_infos().end()) {
            file = it->second.file;
            op = it->second.opcode;
            line = it->second.line;
            col = it->second.col;
        }
        fprintf(f, "\t%s:%d:%d\t%-6s\tbits=%.1f gamma=%.3g\n\t[round=%llu nan=%llu inf=%llu cancel=%llu sens=%llu supp=%llu branch=%llu conv=%llu]\n",
                file.c_str(), line, col, op.c_str(),
                kv.ss->max_bits, kv.ss->max_gamma,
                (unsigned long long)kv.ss->thres_hits,
                (unsigned long long)kv.ss->nan_hits,
                (unsigned long long)kv.ss->inf_hits,
                (unsigned long long)kv.ss->cancellation_hits,
                (unsigned long long)kv.ss->sensitivity_hits,
                (unsigned long long)kv.ss->suppressed_hits,
                (unsigned long long)kv.ss->branch_hits,
                (unsigned long long)kv.ss->conv_hits);
    }
    fprintf(f, "\n");
    fclose(f);
}

void check_cond_error(uint32_t site_id, int err_kind, double gamma, double operand) {
    SiteStats &SS = site_stats()[site_id];
    // CondSite &C = cond_sites[site_id];
    if (err_kind == (int)ErrKind::Cancellation) {
        G.cond_cancellation++;
        SS.cancellation_hits++;
    }
    else {
        G.cond_sensitivity++;
        SS.sensitivity_hits++;
    }
    
    if (gamma > SS.max_gamma) {
        SS.max_gamma = gamma;
        SS.sample_operand = operand;
        SS.worst_kind = (ErrKind)err_kind;
    }
}

// void report_cond_err() {   // called once at exit
//     if (cond_sites.empty()) {
//         printf("\n[condition-number] no sites flagged\n");
//         return;
//     }

//     // collect + sort by max_gamma (fragility ranking)
//     std::vector<std::pair<uint32_t, CondSite>> v(cond_sites.begin(), cond_sites.end());
//     std::sort(v.begin(), v.end(),
//         [](const auto &A, const auto &B){ return A.second.max_gamma > B.second.max_gamma; });

//     printf("\n--- [condition-number report] ---\n");
//     printf("detected=%llu cancellation=%llu sensitivity=%llu untyped=%llu suppressed=%llu\n",
//         (unsigned long long)G.cond_detected,
//         (unsigned long long)G.cond_cancellation,
//         (unsigned long long)G.cond_sensitivity,
//         (unsigned long long)G.cond_untyped,
//         (unsigned long long)G.cond_suppressed);

//     size_t limit = std::min<size_t>(10, v.size());
//     for (size_t i = 0; i < limit; i++) {
//         uint32_t id = v[i].first;
//         const CondSite &C = v[i].second;

//         auto it = site_infos.find(id);
//         if (it != site_infos.end()) {
//             const SiteInfo &S = it->second;
//             printf("[%zu] %s:%d:%d in %s (%s)\n",
//                 i+1, S.file.c_str(), S.line, S.col,
//                 S.function.c_str(), S.opcode.c_str());
//         } else {
//             printf("[%zu] site=%u <no source info>\n", i+1, id);
//         }
//         printf("    kind=%s  max_gamma=%.3e  cancellation_hits=%llu  sensitivity_hits=%llu  (operand=%.6e)\n",
//             C.worst_kind == ErrKind::Cancellation ? "cancellation" : "sensitivity",
//             C.max_gamma,
//             (unsigned long long)C.cancellation_hits,
//             (unsigned long long)C.sensitivity_hits,
//             C.sample_operand);
//     }
// }