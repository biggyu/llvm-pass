#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "fp_condition.h"
#include "fp_debug.h"
#include "fp_runtime_state.h"

static double load_threshold() {
    if (const char *s = std::getenv("FPCHECK_THRESHOLD")) {
        char *end = nullptr;
        double v = std::strtod(s, &end);
        if (end != s && v > 0.0) {
            return v;
        }
        std::fprintf(stderr, "[fpcheck] ignoring invalid FPCHECK_THRESHOLD\"%s\"\n", s);
    }
    return 1e15;
}

static double g_threshold_val = 0.0;
static bool g_threshold_init = false;
double get_g_threshold() {
    if (!g_threshold_init) {
        g_threshold_val = load_threshold();
        g_threshold_init = true;
    }
    return g_threshold_val;
}

double condition_number(uint32_t opraw, double a, double a_Ex, double b, double b_Ex, double aVal, double bVal, uint32_t siteId) {
    SplitGamma splits[2];
    int n = 0;
    double full, Ex = 0;
    // bool aExact = (a == aVal), bExact = (b == bVal);
    double Ea = a == aVal ? 0.0 : std::fabs(a_Ex);
    double Eb = b == bVal ? 0.0 : std::fabs(b_Ex);
    FpOp opcode = (FpOp)opraw;
    switch (opcode) {
        case FpOp::Add:
        case FpOp::Sub: {
            double denom = (opcode == FpOp::Add) ? (a + b) : (a - b);
            double ga = std::fabs(a / denom);
            double gb = std::fabs(b / denom);
            splits[0] = {ga, ErrKind::Cancellation, a == aVal};
            splits[1] = {gb, ErrKind::Cancellation, b == bVal};
            n = 2;
            full = std::max(ga, gb);
            Ex = ga * Ea + gb * Eb;
            break;
        }

        // case FpOp::Mul:
        // case FpOp::Div: {
        //     n = 0;
        //     full = 1.0;
        //     break;
        // }

        // case FpOp::Sqrt: {
        //     n = 0;
        //     full = .5;
        //     break;
        // }
        // case FpOp::Cbrt: {
        //     n = 0;
        //     full = 1/3;
        //     break;
        // }

        case FpOp::Log: {
            double g = std::fabs(1.0 / std::log(a));
            splits[0] = {g, ErrKind::Cancellation, a == aVal};
            n = 1;
            full = g;
            if (a == aVal) {
                Ex = 0.0;
            }
            else {
                Ex = g * Ea;
            }
            printf("%f %f %f %d", full, g, Ex, a == aVal);
            break;
        }
        case FpOp::Exp: {
            double g = std::fabs(a);
            splits[0] = {g, ErrKind::Sensitivity, a == aVal};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Pow: {
            double ga = std::fabs(b);
            double gb = std::fabs(b * std::log(a));
            splits[0] = {ga, ErrKind::Sensitivity, a == aVal};
            splits[1] = {gb, ErrKind::Sensitivity, b == bVal};
            n = 2;
            full = std::max(ga, gb);
            Ex = ga * Ea + gb * Eb;
            break;
        }

        case FpOp::Sin: {
            double g1 = std::fabs(1.0 / std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, a == aVal};
            splits[1] = {g2, ErrKind::Sensitivity, a == aVal};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }
        case FpOp::Cos: {
            double g1 = std::fabs(std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, a == aVal};
            splits[1] = {g2, ErrKind::Sensitivity, a == aVal};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }
        case FpOp::Tan: {
            double g1 = std::fabs(std::tan(a) + 1.0 / std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, a == aVal};
            splits[1] = {g2, ErrKind::Sensitivity, a == aVal};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }

        case FpOp::Acos: {
            double g = std::fabs(a / (std::sqrt(1.0 - a * a) * std::acos(a)));
            splits[0] = {g, ErrKind::Cancellation, a == aVal};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Asin: {
            double g = std::fabs(a / (std::sqrt(1.0 - a * a) * std::asin(a)));
            splits[0] = {g, ErrKind::Cancellation, a == aVal};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Atan: {
            double g = std::fabs(a / ((1.0 + a * a) * std::atan(a)));
            splits[0] = {g, ErrKind::Cancellation, a == aVal};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        
        default: {
            n = 0;
            full = 0;
            Ex = 0;
        }
    }
    if (full > get_g_threshold()) {
        bool anyRealError = false;
        for (int i = 0; i < n; i++) {
            if (!splits[i].exact) {
                anyRealError = true;
                break;
            }
        }
        if (!anyRealError) {
            G.cond_suppressed++;
            site_stats()[siteId].suppressed_hits++;
        }
        else {
            G.cond_detected++;
            bool any_typed = false;
            for (int i = 0; i < n; i++) {
                if(splits[i].exact) continue;
                if(splits[i].value > get_g_threshold()) {
                    check_cond_error(siteId, (int) splits[i].kind, splits[i].value, a);
                    any_typed = true;
                }
            }
            if (!any_typed) {
                G.cond_untyped++;
            }
        }
    }
    return Ex;
}
