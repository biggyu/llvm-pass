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
                        Value *isExact) {
    LLVMContext &Ctx = xhat->getContext();
    
    DSLValues d;
    Value *xd = xhat->getType()->isFloatTy() ? B.CreateFPExt(xhat, rt.DoubleTy, "dsl.xhat") : xhat;
    Value *rd = rhat->getType()->isFloatTy() ? B.CreateFPExt(rhat, rt.DoubleTy, "dsl.rhat") : rhat;
    d.xhat = xd;
    d.rhat = rd;

    Value *absR = B.CreateUnaryIntrinsic(Intrinsic::fabs, rd);
    Value *absX = B.CreateUnaryIntrinsic(Intrinsic::fabs, xd);

    Value *isZero = B.CreateFCmpOEQ(absX, rt.ZeroD);
    Value *pos_inf = ConstantFP::get(rt.DoubleTy, std::numeric_limits<double>::infinity());
    Value *ratio = B.CreateFDiv(absR, absX, "dsl.ratio");
    Value *relerr_comp = B.CreateSelect(isZero, pos_inf, ratio, "dsl.relerr_comp");
    d.relerr = B.CreateSelect(isExact, rt.ZeroD, relerr_comp, "dsl.relerr");

    Value *neg_inf = ConstantFP::get(rt.DoubleTy, -std::numeric_limits<double>::infinity());
    Value *log_abs = B.CreateUnaryIntrinsic(Intrinsic::log2, absX);
    d.ehat = B.CreateSelect(isZero, neg_inf, log_abs, "dsl.ehat");
    
    Value *x_true = B.CreateFAdd(xd, rd, "dsl.x_true");
    d.sign = B.CreateFCmpOLT(x_true, rt.ZeroD, "dsl.sign");

    d.isExact = isExact;
    return d;
}

DSLValues getDSL(IRBuilder<> &B,
                Value *v, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

DSLValues extractDSL(IRBuilder<> &B, Value *v);
