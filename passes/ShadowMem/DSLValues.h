#pragma once
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_fp.h"

using namespace llvm;

struct DSLValues {
    llvm::Value *xhat = nullptr;
    llvm::Value *rhat = nullptr;
    llvm::Value *sign = nullptr;
    llvm::Value *ehat = nullptr;
    llvm::Value *isExact = nullptr;
    llvm::Value *relerr = nullptr;
};

inline DSLValues makeDSL(IRBuilder<> &B, 
                        Value *xhat, Value *rhat, 
                        utils::RuntimeFns &rt,
                        // Constant *ZeroF, Constant *ZeroD, 
                        bool isExact) {
    LLVMContext &Ctx = xhat->getContext();
    
    DSLValues d;
    d.xhat = xhat;
    d.rhat = rhat;
    if (isExact) {
        d.relerr = rt.ZeroD;
    }
    else {
        Value *absR = B.CreateUnaryIntrinsic(Intrinsic::fabs, rhat);
        Value *absX = B.CreateUnaryIntrinsic(Intrinsic::fabs, xhat);

        Value *isZero = B.CreateFCmpOEQ(absX, rt.ZeroD);
        Value *inf = ConstantFP::get(rt.DoubleTy, std::numeric_limits<double>::infinity());
        Value *ratio = B.CreateFDiv(absR, absX);
        d.relerr = B.CreateSelect(isZero, inf, ratio, "dsl.relerr");
    }
    Value *x = B.CreateFAdd(xhat, rhat, "dsl.x");
    Value *abs_x = B.CreateUnaryIntrinsic(Intrinsic::fabs, x);

    Value *isZero = B.CreateFCmpOEQ(abs_x, rt.ZeroD);
    Value *log_abs = B.CreateUnaryIntrinsic(Intrinsic::log2, abs_x);
    
    Value *negInf = ConstantFP::get(rt.DoubleTy, -std::numeric_limits<double>::infinity());

    d.ehat = B.CreateSelect(isZero, negInf, log_abs, "dsl.ehat");
    d.sign = B.CreateFCmpOLT(x, rt.ZeroD, "dsl.sign");
    d.isExact = ConstantInt::get(rt.BoolTy, isExact);
    return d;
}

DSLValues getDSL(IRBuilder<> &B,
                Value *v, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

DSLValues extractDSL(IRBuilder<> &B, Value *v);
