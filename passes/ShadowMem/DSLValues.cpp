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
    d.xhat = v;
    d.rhat = rt.ZeroD;
    llvm::LLVMContext &ctx = v->getContext();
    d.sign = ConstantInt::getFalse(ctx);
    Value *abs = B.CreateUnaryIntrinsic(Intrinsic::fabs, v);
    //? Only float and double?
    Value *isZero = v->getType()->isFloatTy() ? B.CreateFCmpOEQ(abs, rt.ZeroF) : B.CreateFCmpOEQ(abs, rt.ZeroD);
    // if (v->getType()->isFloatTy()) {
    //     isZero = B.CreateFCmpOEQ(abs, rt.ZeroF);
    // }
    // else if (v->getType()->isDoubleTy()) {
    //     isZero = B.CreateFCmpOEQ(abs, rt.ZeroD);
    // }

    Value *log_abs = B.CreateUnaryIntrinsic(Intrinsic::log2, abs);
    Value *negInf = ConstantFP::get(rt.DoubleTy, -std::numeric_limits<double>::infinity());

    d.ehat = B.CreateSelect(isZero, negInf, log_abs, "dsl.ehat");
    d.isExact = ConstantInt::getTrue(ctx);
    d.relerr = rt.ZeroD;
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