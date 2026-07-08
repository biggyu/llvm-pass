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
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleExternal(CallInst *CI, utils::RuntimeFns &rt,
                utils::RuntimeMPFRFns &rt_mpfr,
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleUnary(UnaryOperator *UO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleBinary(BinaryOperator *BO, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleFCmp(FCmpInst *FC, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleFPToSI(FPToSIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

bool handleFPToUI(FPToUIInst *CI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);
                
bool handleSIToFP(SIToFPInst *SI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);
                
bool handleUIToFP(UIToFPInst *UI, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);
