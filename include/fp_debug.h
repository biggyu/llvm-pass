#pragma once
#include <cstddef>
#ifdef __cplusplus
extern "C" {
#endif

// void register_fp_site(int site_id, const char* function, const char *file, int line, int col, const char* opcode);

void check_conv_si(int val, double src, double src_err);
void check_conv_ui(size_t val, double src, double src_err);

void check_branch(double a, double da, double b, double db, size_t pred, bool computed_res);

void check_error(double x, double dx, int metric);
// void check_error_double(double x, double dx, int site_id, int metric);
// void check_error_float(float x, double dx, int site_id, int metric);

void report_debug_summary();

void report_cond_err(int site_id, int err_kind, double gamma, double operand);

#ifdef __cplusplus
}
#endif