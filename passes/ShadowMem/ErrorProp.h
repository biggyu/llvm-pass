#pragma once

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_mpfr.h"

using namespace llvm;

bool handleIntrinsic(IntrinsicInst *II, Constant *ZeroF, Constant *ZeroD, 
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleExternal(CallInst *CI, Constant *ZeroF, Constant *ZeroD, 
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleBinary(Instruction *BO, Constant *ZeroF, Constant *ZeroD,
                DenseMap<const Value*, Value*> &ErrorMap);