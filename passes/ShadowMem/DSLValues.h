#pragma once
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_fp.h"

using namespace llvm;

struct DSLValues {
    llvm::Value *xhat = nullptr;
    llvm::Value *rhat = nullptr;
    llvm::Value *sign = nullptr;
    llvm::Value *isExact = nullptr;
    llvm::Value *ehat = nullptr;
    llvm::Value *error = nullptr;
};

inline DSLValues makeDSL(Value *xhat, Value *error, Constant *ZeroD) {
    DSLValues d;
    d.xhat = xhat;
    d.error = error;
    d.ehat = ZeroD;
    d.rhat = ZeroD;
    d.sign = ConstantInt::getFalse(xhat->getContext());
    d.isExact = ConstantInt::getTrue(xhat->getContext());
    return d;
}

DSLValues getDSL(Value *v, Constant *ZeroD, 
                    DenseMap<const Value*, DSLValues> &DSLMap);

DSLValues extractDSL(IRBuilder<> &B, Value *v);
