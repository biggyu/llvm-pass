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
        d.fpval = rt.ZeroD;
        d.relerr = rt.ZeroD;
    }
    else {
        Value *xd = v->getType()->isFloatTy() ? B.CreateFPExt(v, rt.DoubleTy, "dsl.xhat") : v;
        d.xhat = xd;
        
        d.rhat = rt.ZeroD;
        d.fpval = xd;
        d.relerr = rt.ZeroD;
    }
    return d;
}

DSLValues extractDSL(IRBuilder<> &B, Value *v) {
    DSLValues d;
    d.xhat = B.CreateExtractValue(v, {1}, "xhat");
    d.rhat = B.CreateExtractValue(v, {2}, "rhat");
    d.fpval = B.CreateExtractValue(v, {3}, "fpval");
    d.relerr = B.CreateExtractValue(v, {4}, "relerr");
    return d;
}