#pragma once

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_fp.h"
#include "decls_mpfr.h"

using namespace llvm;

bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleUnary(UnaryOperator *UO, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleBinary(BinaryOperator *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleFCmp(FCmpInst *FC, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleFPToSI(FPToSIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleFPToUI(FPToUIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);
                
bool handleSIToFP(SIToFPInst *SI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);
                
bool handleUIToFP(UIToFPInst *UI, utils::RuntimeFns &rt,
                DenseMap<const Value*, Value*> &ErrorMap);