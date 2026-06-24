#pragma once

#include "llvm/IR/Instruction.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_mpfr.h"
#include "decls_fp.h"
#include "DSLValues.h"

using namespace llvm;

bool handleStore(StoreInst *SI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap);
bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap);