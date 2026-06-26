#include "fp_debug.h"
#include "fp_condition.h"
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

static uint64_t total_checks_double = 0;
static uint64_t total_checks_float = 0;

enum class ErrorClass : uint8_t {
    Exact,      // dx == 0
    Normal,     // 0 < |dx| < |x|
    TotalLoss,  // |dx| >= |x| — "more than precision bits incorrect"
    NaNOrInf,   // x or dx is non-finite
    XZero       // x == 0 but dx != 0 — special: any nonzero dx is total loss
};

struct SiteStats {
    uint64_t cnt = 0;
    uint64_t finite_cnt = 0;

    double sum_bits = 0.0;
    double max_bits = 0.0;

    double sum_err = 0.0;
    double max_relerr = 0.0;
    double max_abs_dx = 0.0;

    uint64_t warn_4 = 0;
    uint64_t warn_8 = 0;
    uint64_t warn_16 = 0;
    uint64_t warn_prec = 0;

    uint64_t exact = 0;
    uint64_t normal = 0;
    uint64_t total_loss = 0;
    uint64_t nan_or_inf = 0;
    uint64_t xzero = 0;

    double sample_x = 0.0;
    double sample_dx = 0.0;

    // double sample_xzero_x = 0.0;
    // double sample_xzero_dx = 0.0;

    // condition-number aggregation
    uint64_t cond_warn_cancellation = 0.0;
    uint64_t cond_warn_sensitivity = 0.0;
    
    double max_gamma = 0.0;
    double sample_gamma_x = 0.0;
};

struct SiteInfo {
    std::string function;
    std::string file;
    int line = 0;
    int col = 0;
    std::string opcode;
};

static std::unordered_map<int, SiteStats> double_sites;
static std::unordered_map<int, SiteStats> float_sites;
static std::unordered_map<int, SiteInfo> site_infos;

template <typename T>
static ErrorClass classify(T x, T dx) {
    if (!std::isfinite(x) || !std::isfinite(dx)) {
        return ErrorClass::NaNOrInf;
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

extern "C" void register_fp_site(int site_id, const char* function, const char *file, int line, int col, const char* opcode) {
    if (site_infos.find(site_id) != site_infos.end()) {
        return;
    }
    SiteInfo info;
    info.function = function ? function : "<unknown>";
    info.file = file ? file : "<unknown>";
    info.line = line;
    info.col = col;
    info.opcode = opcode ? opcode : "<unknown>";
    site_infos[site_id] = std::move(info);
}

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
static void check_error_impl(T x, T dx, int site_id, int metric, uint64_t &total_checks, std::unordered_map<int, SiteStats> &sites) {
    ErrorClass errcls = classify<T>(x, dx);
    double bits = incorrect_bits<T>(x, dx, metric);
    double relerr = relative_error<T>(x, dx);
    double precision = precision_bits<T>();

    total_checks++;

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
        case ErrorClass::NaNOrInf:
            S.nan_or_inf++;
            break;
        case ErrorClass::XZero:
            S.xzero++;
            break;
    }

    bool finite_metric = errcls != ErrorClass::XZero && errcls != ErrorClass::NaNOrInf && std::isfinite(bits) && std::isfinite(relerr);
    if (!finite_metric) {
        S.warn_4++;
        S.warn_8++;
        S.warn_16++;
        S.warn_prec++;

        S.sample_xzero_x = x;
        S.sample_xzero_dx = dx;
        return;
    }
    S.finite_cnt++;
    S.sum_bits += bits;
    S.sum_err += relerr;
    if (bits >= S.max_bits) {
        S.sample_x = x;
        S.sample_dx = dx;
        S.max_bits = bits;
    }
    S.max_relerr = S.max_relerr > relerr ? S.max_relerr : relerr;
    S.max_abs_dx = S.max_abs_dx > std::fabs(dx) ? S.max_abs_dx : std::fabs(dx);
    if (bits >= 4.0) {
        S.warn_4++;
    }
    if (bits >= 8.0) {
        S.warn_8++;
    }
    if (bits >= 16.0) {
        S.warn_16++;
    }
    if (bits >= 53.0) {
        S.warn_prec++;
    }
}

extern "C" void check_error_double(double x, double dx, int site_id, int metric) {
    return check_error_impl<double>(x, dx, site_id, metric, total_checks_double, double_sites);
}

extern "C" void check_error_float(float x, float dx, int site_id, int metric) {
    return check_error_impl<float>(x, dx, site_id, metric, total_checks_float, float_sites);
}

template <typename T>
static void report_top_impl(std::unordered_map<int, SiteStats> &site_map) {
    std::vector<std::pair<int, SiteStats>> sites;

    for (const auto &KV : site_map) {
        sites.push_back(KV);
    }

    std::sort(sites.begin(), sites.end(),
        [](const auto &A, const auto &B) {
            const SiteStats &SA = A.second;
            const SiteStats &SB = B.second;

            return SA.max_bits > SB.max_bits;
        }
    );

    size_t limit = std::min<size_t>(10, sites.size());

    printf("\nTop %s sites by numerical severity::\n", precision_bits<T>() == 24.0 ? "float" : "double");

    for (size_t i = 0; i < limit; i++) {
        int site_id = sites[i].first;
        
        auto It = site_infos.find(site_id);
        if (It != site_infos.end()) {
            const SiteInfo &Info = It->second;

            printf("[%zu] site=%d %s:%d:%d function=%s opcode=%s\n",
                i + 1,
                site_id,
                Info.file.c_str(),
                Info.line,
                Info.col,
                Info.function.c_str(),
                Info.opcode.c_str());
        } else {
            printf("[%zu] site=%d <no source info>\n", i + 1, site_id);
        }

        const SiteStats &S = sites[i].second;

        double avg_bits = S.cnt ? S.sum_bits / S.finite_cnt : 0.0;
        double avg_relerr = S.cnt ? S.sum_err / S.finite_cnt : 0.0;

        printf("    site=%d count=%llu finite count=%llu max_bits=%.2f avg_bits=%.2f "
               "max_relerr=%.3e avg_relerr=%.3e max_abs_dx=%.3e\n",
               site_id,
               (unsigned long long)S.cnt,
               (unsigned long long)S.finite_cnt,
               S.max_bits,
               avg_bits,
               S.max_relerr,
               avg_relerr,
               S.max_abs_dx);

        printf("    warnings: >4=%llu >8=%llu >16=%llu >precision=%llu\n",
               (unsigned long long)S.warn_4,
               (unsigned long long)S.warn_8,
               (unsigned long long)S.warn_16,
               (unsigned long long)S.warn_prec);

        printf("    classes: exact=%llu normal=%llu total_loss=%llu nan_inf=%llu xzero=%llu\n",
               (unsigned long long)S.exact,
               (unsigned long long)S.normal,
               (unsigned long long)S.total_loss,
               (unsigned long long)S.nan_or_inf,
               (unsigned long long)S.xzero);

        printf("    sample: x=%.17e dx=%.17e\n",
               S.sample_x,
               S.sample_dx);
    }
}

static void report_top_double(std::unordered_map<int, SiteStats> &site_map) {
    return report_top_impl<double>(site_map);
}

static void report_top_float(std::unordered_map<int, SiteStats> &site_map) {
    return report_top_impl<float>(site_map);
}

extern "C" void report_debug_summary() {
    printf("--- [fp debug summary] ---\n");

    printf("double checks=%llu\n", (unsigned long long)total_checks_double);
    report_top_double(double_sites);

    printf("float checks=%llu\n", (unsigned long long)total_checks_float);
    report_top_float(float_sites);

}

static void report_cond_err(int site_id, int err_kind, double xhat, double value) {
    
}