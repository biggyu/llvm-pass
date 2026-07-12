#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"
#include "DSLValues.h"

DSLValues getDSL(IRBuilder<> &B,
                Value *v, utils::RuntimeFns &rt,
                DenseMap<const Value*, DSLValues> &DSLMap) {
    auto it = DSLMap.find(v);
    if (it != DSLMap.end()) {
        return it->second;
    }

    DSLValues d;
    if (!v->getType()->isFloatTy() && !v->getType()->isDoubleTy()) {
        d.xhat = rt.ZeroD;
        d.rhat = rt.ZeroD;
        d.sign = rt.FalseVal;
        d.ehat = ConstantFP::get(rt.DoubleTy, -std::numeric_limits<double>::infinity());
        d.isExact = rt.TrueVal;
        d.relerr = rt.ZeroD;
    }
    else {
        Value *xd = v->getType()->isFloatTy() ? B.CreateFPExt(v, rt.DoubleTy, "dsl.xhat") : v;
        d.xhat = xd;
        
        d.rhat = rt.ZeroD;
        d.sign = B.CreateFCmpOLT(xd, rt.ZeroD, "dsl.sign");
    
        Value *absX = B.CreateUnaryIntrinsic(Intrinsic::fabs, xd);
        Value *isZero = B.CreateFCmpOEQ(absX, rt.ZeroD);
    
        Value *neg_inf = ConstantFP::get(rt.DoubleTy, -std::numeric_limits<double>::infinity());
        Value *log_abs = B.CreateUnaryIntrinsic(Intrinsic::log2, absX);
        d.ehat = B.CreateSelect(isZero, neg_inf, log_abs, "dsl.ehat");
    
        d.isExact = rt.TrueVal;
        d.relerr = rt.ZeroD;
    }
    return d;
}

DSLValues extractDSL(IRBuilder<> &B, Value *v) {
    DSLValues d;
    d.xhat = B.CreateExtractValue(v, {1}, "xhat");
    d.rhat = B.CreateExtractValue(v, {2}, "rhat");
    d.sign = B.CreateExtractValue(v, {3}, "sign");
    d.ehat = B.CreateExtractValue(v, {4}, "ehat");
    d.isExact = B.CreateExtractValue(v, {5}, "isExact");
    d.relerr = B.CreateExtractValue(v, {6}, "relerr");
    return d;
}