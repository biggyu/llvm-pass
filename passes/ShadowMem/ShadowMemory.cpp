#include "ShadowMemory.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

// static DSLValues getDSL(Value *v, utils::RuntimeFns &rt, 
//                     DenseMap<const Value*, DSLValues> &DSLMap) {
//     auto it = DSLMap.find(v);
//     if (it != DSLMap.end()) {
//         return it->second;
//     }
//     DSLValues d;
//     bool isD = v->getType()->isDoubleTy();
//     Constant *zfp = v->getType()->isDoubleTy() ? rt.ZeroD : rt.ZeroF;
//     d.xhat = v;
//     d.rhat = zfp;
//     d.error = zfp;
//     d.sign = ConstantInt::getFalse(rt.BoolTy);
//     d.isExact = ConstantInt::getTrue(rt.BoolTy);
//     d.ehat = zfp;
//     return d;
// }

bool handleStore(StoreInst *SI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap) {
    llvm::Value *val = SI->getValueOperand();
    if (!val->getType()->isDoubleTy() && !val->getType()->isFloatTy()) {
        return false;
    }
    if (SI->isVolatile() || SI->isAtomic()) {
        return false;
    }
    llvm::Value *ptr = SI->getPointerOperand();
    DSLValues dsl = getDSL(val, rt, DSLMap);

    IRBuilder<> AfterSI(SI->getNextNode());

    // AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dsl.rhat, dsl.error, dsl.sign, dsl.isExact, dsl.ehat});
    // AfterSI.CreateCall(rt.ShadowStore, {ptr, val, dsl.rhat, dsl.sign, dsl.isExact, dsl.ehat, dsl.error});
    if (val->getType()->isDoubleTy()) {
        AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dsl.rhat, dsl.error, dsl.sign, dsl.isExact, dsl.ehat});
    }
    else {
        AfterSI.CreateCall(rt.ShadowStoreF, {ptr, val, dsl.rhat, dsl.error, dsl.sign, dsl.isExact, dsl.ehat});
    }
    return true;
}

bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap) {
    if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
        return false;
    }
    if (LI->isVolatile() || LI->isAtomic()) {
        return false;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    IRBuilder<> AfterLI(LI->getNextNode());
    llvm::Value *entryPtr = AfterLI.CreateCall(rt.ShadowLoad, {ptr});
    DSLMap[LI] = extractDSL(AfterLI, entryPtr);
    return true;
}