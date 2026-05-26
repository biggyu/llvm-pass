#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/CommandLine.h"
#include "decls_fp.h"

using namespace llvm;

extern cl::opt<bool> EnableDebugChecks;

extern cl::opt<int> DebugMetrics;

bool insertCheckError(IRBuilder<> &B, Value *x, Value *dx, Instruction *Site, utils::RuntimeFns &rt);
