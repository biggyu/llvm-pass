#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"
#include "DSLValues.h"

DSLValues getDSL(Value *v, utils::RuntimeFns &rt, 
                    DenseMap<const Value*, DSLValues> &DSLMap) {
    auto it = DSLMap.find(v);
    if (it != DSLMap.end()) {
        return it->second;
    }
    DSLValues d;
    bool isD = v->getType()->isDoubleTy();
    Constant *zfp = v->getType()->isDoubleTy() ? rt.ZeroD : rt.ZeroF;
    d.xhat = v;
    d.rhat = zfp;
    d.error = zfp;
    llvm::LLVMContext &ctx = v->getContext();
    d.sign = ConstantInt::getFalse(ctx);
    d.isExact = ConstantInt::getTrue(ctx);
    d.ehat = zfp;
    return d;
}

DSLValues extractDSL(IRBuilder<> &B, Value *v) {
    DSLValues d;
    d.xhat = B.CreateExtractValue(v, {1}, "xhat");
    d.rhat = B.CreateExtractValue(v, {2}, "rhat");
    d.sign = B.CreateExtractValue(v, {3}, "sign");
    d.isExact = B.CreateExtractValue(v, {4}, "isExact");
    d.ehat = B.CreateExtractValue(v, {5}, "ehat");
    d.error = B.CreateExtractValue(v, {6}, "error");
    return d;
}