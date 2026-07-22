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
    return 1e16;
}

double g_threshold = load_threshold();

double condition_number(uint32_t opraw, double a, double a_Ex, double b, double b_Ex, bool aExact, bool bExact, uint32_t siteId) {
    // FpOp opcode = (FpOp)opraw;
    SplitGamma splits[2];
    int n = 0;
    double full, Ex = 0;
    double Ea = aExact ? 0.0 : std::fabs(a_Ex);
    double Eb = bExact ? 0.0 : std::fabs(b_Ex);
    FpOp opcode = (FpOp)opraw;
    switch (opcode) {
        case FpOp::Add:
        case FpOp::Sub: {
            double denom = (opcode == FpOp::Add) ? (a + b) : (a - b);
            double ga = std::fabs(a / denom);
            double gb = std::fabs(b / denom);
            splits[0] = {ga, ErrKind::Cancellation, aExact};
            splits[1] = {gb, ErrKind::Cancellation, bExact};
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
            splits[0] = {g, ErrKind::Cancellation, aExact};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Exp: {
            double g = std::fabs(a);
            splits[0] = {g, ErrKind::Sensitivity, aExact};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Pow: {
            double ga = std::fabs(b);
            double gb = std::fabs(b * std::log(a));
            splits[0] = {ga, ErrKind::Sensitivity, aExact};
            splits[1] = {gb, ErrKind::Sensitivity, bExact};
            n = 2;
            full = std::max(ga, gb);
            Ex = ga * Ea + gb * Eb;
            break;
        }

        case FpOp::Sin: {
            double g1 = std::fabs(1.0 / std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, aExact};
            splits[1] = {g2, ErrKind::Sensitivity, aExact};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }
        case FpOp::Cos: {
            double g1 = std::fabs(std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, aExact};
            splits[1] = {g2, ErrKind::Sensitivity, aExact};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }
        case FpOp::Tan: {
            double g1 = std::fabs(std::tan(a) + 1.0 / std::tan(a));
            double g2 = std::fabs(a);
            splits[0] = {g1, ErrKind::Cancellation, aExact};
            splits[1] = {g2, ErrKind::Sensitivity, aExact};
            n = 2;
            full = g1 * g2;
            Ex = full * Ea;
            break;
        }

        case FpOp::Acos: {
            double g = std::fabs(a / (std::sqrt(1.0 - a * a) * std::acos(a)));
            splits[0] = {g, ErrKind::Cancellation, aExact};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Asin: {
            double g = std::fabs(a / (std::sqrt(1.0 - a * a) * std::asin(a)));
            splits[0] = {g, ErrKind::Cancellation, aExact};
            n = 1;
            full = g;
            Ex = g * Ea;
            break;
        }
        case FpOp::Atan: {
            double g = std::fabs(a / ((1.0 + a * a) * std::atan(a)));
            splits[0] = {g, ErrKind::Cancellation, aExact};
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
    if (full > g_threshold) {
        bool cond = (n == 1) ? splits[0].exact : splits[0].exact && splits[1].exact;
        if (cond) {
            G.cond_suppressed++;
        }
        else {
            G.cond_detected++;
            bool any_typed = false;
            for (int i = 0; i < n; i++) {
                if(splits[i].exact) continue;
                if(splits[i].value > g_threshold) {
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
