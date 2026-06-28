#pragma once

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_fp.h"
#include "decls_mpfr.h"
#include "DSLValues.h"

using namespace llvm;

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &ErrorMap);

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &ErrorMap);

bool handleBinary(Instruction *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &ErrorMap);
