#include "fp_condition.h"
#include "fp_debug.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>

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

// template <typename T>
// double condition_number_impl(uint32_t opraw, SplitGamma *splits, int *nsplits, T a, T da, T b, T db, bool aExact, bool bExact, uint32_t siteId) {
//     FpOp opcode = (FpOp)opraw;
//     switch (opcode) {
//         case FpOp::Add:
//         case FpOp::Sub: {
//             double denom = (opcode == FpOp::Add) ? (a + b) : (a - b);
//             double ga = std::fabs(a / denom);
//             double gb = std::fabs(b / denom);
//             splits[0] = {ga, ErrKind::Cancellation, aExact};
//             splits[1] = {gb, ErrKind::Cancellation, bExact};
//             *nsplits = 2;
//             return std::max(ga, gb);
//         }

//         case FpOp::Mul:
//         case FpOp::Div: {
//             *nsplits = 0;
//             return 1.0;
//         }

//         case FpOp::Sqrt: {
//             *nsplits = 0;
//             return .5;
//         }
//         case FpOp::Cbrt: {
//             *nsplits = 0;
//             return 1/3;
//         }

//         case FpOp::Log: {
//             double g = std::fabs(1.0 / std::log(a));
//             splits[0] = {g, ErrKind::Cancellation, aExact};
//             *nsplits = 1;
//             return g;
//         }
//         case FpOp::Exp: {
//             double g = std::fabs(a);
//             splits[0] = {g, ErrKind::Sensitivity, aExact};
//             *nsplits = 1;
//             return g;
//         }
//         case FpOp::Pow: {
//             double ga = std::fabs(b);
//             double gb = std::fabs(b * std::log(a));
//             splits[0] = {ga, ErrKind::Sensitivity, aExact};
//             splits[1] = {gb, ErrKind::Sensitivity, bExact};
//             *nsplits = 2;
//             return std::max(ga, gb);
//         }

//         case FpOp::Sin: {
//             double g1 = std::fabs(1.0 / std::tan(a));
//             double g2 = std::fabs(a);
//             splits[0] = {g1, ErrKind::Cancellation, aExact};
//             splits[1] = {g2, ErrKind::Sensitivity, aExact};
//             *nsplits = 2;
//             return g1 * g2;
//         }
//         case FpOp::Cos: {
//             double g1 = std::fabs(std::tan(a));
//             double g2 = std::fabs(a);
//             splits[0] = {g1, ErrKind::Cancellation, aExact};
//             splits[1] = {g2, ErrKind::Sensitivity, aExact};
//             *nsplits = 2;
//             return g1 * g2;
//         }
//         case FpOp::Tan: {
//             double g1 = std::fabs(std::tan(a) + 1.0 / std::tan(a));
//             double g2 = std::fabs(a);
//             splits[0] = {g1, ErrKind::Cancellation, aExact};
//             splits[1] = {g2, ErrKind::Sensitivity, aExact};
//             *nsplits = 2;
//             return g1 * g2;
//         }

//         case FpOp::Acos: {
//             double g = std::fabs(a / std::sqrt(1 - std::pow(a, 2) * std::acos(a)));
//             splits[0] = {g, ErrKind::Cancellation, aExact};
//             *nsplits = 1;
//             return g;
//         }
//         case FpOp::Asin: {
//             double g = std::fabs(a / std::sqrt(1 - std::pow(a, 2) * std::asin(a)));
//             splits[0] = {g, ErrKind::Cancellation, aExact};
//             *nsplits = 1;
//             return g;
//         }
//         case FpOp::Atan: {
//             double g = a / ((1 + std::pow(a, 2)) * std::atan(a));
//             splits[0] = {g, ErrKind::Cancellation, aExact};
//             *nsplits = 1;
//             return g;
//         }
//     }
// }

double condition_number(uint32_t opraw, double a, double a_Ex, double b, double b_Ex, bool aExact, bool bExact, uint32_t siteId) {
    // FpOp opcode = (FpOp)opraw;
    SplitGamma splits[2];
    int n = 0;
    // double full = condition_number_impl<double>(opraw, splits, &n, a, da, b, db, aExact, bExact, siteId);
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
        for (int i = 0; i < n; i++) {
            if(splits[i].exact) continue;
            if(splits[i].value > g_threshold) {
                // TODO: report function
                // report_cond_err(siteId, (int) splits[i].kind, splits[i].value, a);
            }
        }
    }
    return Ex;
}
// void condition_number_double(uint32_t opraw, double a, double da, double b, double db, bool aExact, bool bExact, uint32_t siteId) {
//     // FpOp opcode = (FpOp)opraw;
//     SplitGamma splits[2];
//     int n = 0;
//     double full = condition_number_impl<double>(opraw, splits, &n, a, da, b, db, aExact, bExact, siteId);
//     //TODO: g_threshold
//     if (full > g_threshold) {
//         for (int i = 0; i < n; i++) {
//             if(splits[i].exact) continue;
//             if(splits[i].value > g_threshold) {
//                 report_cond_err(siteId, (int) splits[i].kind, splits[i].value, a);
//             }
//         }
//     }
// }
// void condition_number_float(uint32_t opraw, float a, float da, float b, float db, bool aExact, bool bExact, uint32_t siteId) {
//     // FpOp opcode = (FpOp)opraw;
//     SplitGamma splits[2];
//     int n = 0;
//     double full = condition_number_impl<float>(opraw, splits, &n, a, da, b, db, aExact, bExact, siteId);
//     if (full > g_threshold) {
//         for (int i = 0; i < n; i++) {
//             if(splits[i].exact) continue;
//             if(splits[i].value > g_threshold) {
//                 report_cond_err(siteId, (int) splits[i].kind, splits[i].value, a);
//             }
//         }
//     }
// }