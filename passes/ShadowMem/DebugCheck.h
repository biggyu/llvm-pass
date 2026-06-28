#pragma once

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
                    const DSLValues &xDsl, 
                    Instruction *Site, FpOp opcode, 
                    utils::RuntimeFns &rt);
void insertReportDebugSummary(Module &M, utils::RuntimeFns &rt);
// void registerFPSite(IRBuilder<> &B, Instruction *I, utils::RuntimeFns &rt);