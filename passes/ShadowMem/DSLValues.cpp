#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"
#include "DSLValues.h"

DSLValues getDSL(Value *v, Constant *ZeroD, 
                    DenseMap<const Value*, DSLValues> &DSLMap) {
    auto it = DSLMap.find(v);
    if (it != DSLMap.end()) {
        return it->second;
    }
    DSLValues d;
    d.xhat = v;
    d.rhat = ZeroD;
    d.error = ZeroD;
    llvm::LLVMContext &ctx = v->getContext();
    d.sign = ConstantInt::getFalse(ctx);
    d.isExact = ConstantInt::getTrue(ctx);
    d.ehat = ZeroD;
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