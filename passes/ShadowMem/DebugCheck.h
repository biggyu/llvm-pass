#pragma once
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"
#include "decls_fp.h"
#include "../runtime/fp_condition.h"
#include "DSLValues.h"

using namespace llvm;

extern cl::opt<bool> EnableDebugChecks;
extern cl::opt<bool> EnableDebugAutoReport;

extern cl::opt<int> DebugMetrics;

bool insertCheckError(IRBuilder<> &B,
                    const DSLValues &aDsl, 
                    const DSLValues &bDsl, 
                    DSLValues &xDsl, 
                    Instruction *Site, FpOp opcode, 
                    utils::RuntimeFns &rt);

void insertReportDebugSummary(Module &M, utils::RuntimeFns &rt);
// void insertCancellationCheck(IRBuilder<> &B, Value *opr0, Value *opr1, Value *x, Instruction *Site, utils::RuntimeFns &rt);
// void registerFPSite(IRBuilder<> &B, Instruction *I, utils::RuntimeFns &rt);

inline bool isRuntimeFunction(const Function &F) {
    StringRef N = F.getName();
    return N == "shadow_store_double" ||
           N == "shadow_store_float"  ||
           N == "shadow_load_double"  ||
           N == "shadow_load_float"   ||
           N == "shadow_stack_push"   ||
           N == "shadow_stack_pop"    ||
           N == "check_conv_ui"       ||
           N == "check_conv_si"       ||
           N == "check_branch"        ||
           N == "check_error"         ||
           N == "report_debug_summary";
}
