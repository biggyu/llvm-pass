#pragma once
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "decls_fp.h"

using namespace llvm;

struct DSLValues {
    Value *xhat = nullptr;
    Value *rhat = nullptr;
    Value *fpval = nullptr;
    Value *relerr = nullptr;
};

inline DSLValues makeDSL(IRBuilder<> &B, 
                        Value *xhat, Value *rhat, 
                        utils::RuntimeFns &rt,
                        Value *fpval, Value *isExact) {
    LLVMContext &Ctx = xhat->getContext();
    
    DSLValues d;
    Value *xd = xhat->getType()->isFloatTy() ? B.CreateFPExt(xhat, rt.DoubleTy, "dsl.xhat") : xhat;
    Value *rd = rhat->getType()->isFloatTy() ? B.CreateFPExt(rhat, rt.DoubleTy, "dsl.rhat") : rhat;
    Value *fpd = fpval->getType()->isFloatTy() ? B.CreateFPExt(fpval, rt.DoubleTy, "dsl.fpval") : fpval;
    d.xhat = xd;
    d.rhat = rd;
    d.fpval = fpval;

    Value *absR = B.CreateUnaryIntrinsic(Intrinsic::fabs, rd);
    Value *absX = B.CreateUnaryIntrinsic(Intrinsic::fabs, xd);

    Value *isZero = B.CreateFCmpOEQ(absX, rt.ZeroD);
    Value *pos_inf = ConstantFP::get(rt.DoubleTy, std::numeric_limits<double>::infinity());
    Value *ratio = B.CreateFDiv(absR, absX, "dsl.ratio");
    Value *relerr_comp = B.CreateSelect(isZero, pos_inf, ratio, "dsl.relerr_comp");
    d.relerr = B.CreateSelect(isExact, rt.ZeroD, relerr_comp, "dsl.relerr");

    return d;
}

DSLValues getDSL(IRBuilder<> &B,
                Value *v, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap);

DSLValues extractDSL(IRBuilder<> &B, Value *v);
