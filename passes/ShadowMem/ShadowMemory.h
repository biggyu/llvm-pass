#pragma once

#include "llvm/IR/Instruction.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_mpfr.h"
#include "decls_fp.h"

using namespace llvm;

bool handleStore(StoreInst *SI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap);
                
bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap);

bool handleReturn(ReturnInst *RI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap);