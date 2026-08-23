#pragma once
#include <cstdint>
#include <cstddef>
#ifdef __cplusplus
extern "C" {
#endif

void register_fp_site(int site_id, const char* function, const char *file, int line, int col, const char* opcode);

void check_conv_si(int val, double src, double src_err, uint32_t site_id);
void check_conv_ui(size_t val, double src, double src_err, uint32_t site_id);

void check_branch(double a, double da, double b, double db, size_t pred, bool computed_res, uint32_t site_id);

void check_error(double x, double dx, uint32_t site_id, int metric);

void report_debug_summary();

void check_cond_error(uint32_t site_id, int err_kind, double gamma, double operand);

// void report_cond_error();

#ifdef __cplusplus
}
#endif