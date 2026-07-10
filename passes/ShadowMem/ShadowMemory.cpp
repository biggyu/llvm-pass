#include "ShadowMemory.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

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
    IRBuilder<> AfterSI(SI->getNextNode());
    
    DSLValues dsl = getDSL(AfterSI, val, rt, DSLMap);
    // AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dsl.rhat, dsl.error, dsl.sign, dsl.isExact, dsl.ehat});
    // AfterSI.CreateCall(rt.ShadowStore, {ptr, val, dsl.rhat, dsl.sign, dsl.isExact, dsl.ehat, dsl.error});
    if (val->getType()->isDoubleTy()) {
        AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dsl.rhat, dsl.sign, dsl.ehat, dsl.isExact, dsl.relerr});
    }
    else {
        AfterSI.CreateCall(rt.ShadowStoreF, {ptr, val, dsl.rhat, dsl.sign, dsl.ehat, dsl.isExact, dsl.relerr});
    }
    return true;
}

bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap) {
    if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterLI(LI->getNextNode());
    if (LI->isVolatile() || LI->isAtomic()) {
        DSLMap[LI] = getDSL(AfterLI, LI, rt, DSLMap);
        return true;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    // llvm::Value *entryPtr = AfterLI.CreateCall(rt.ShadowLoad, {ptr});
    
    llvm::Value *entryPtr = LI->getType()->isDoubleTy() ? AfterLI.CreateCall(rt.ShadowLoadD, {ptr, LI}) : AfterLI.CreateCall(rt.ShadowLoadF, {ptr, LI});
    // if (LI->getType()->isDoubleTy()) {
        //     dx = AfterLI.CreateCall(rt.ShadowLoadD, {ptr});
        // }
        // else {
        //     dx = AfterLI.CreateCall(rt.ShadowLoadF, {ptr});
        // }
    DSLMap[LI] = extractDSL(AfterLI, entryPtr);
    return true;

    
}

bool handleReturn(ReturnInst *RI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap) {
    Value *ret = RI->getReturnValue();
    if (!ret || (!ret->getType()->isDoubleTy() && !ret->getType()->isFloatTy())) {
        return false;
    }
    IRBuilder<> B(RI);
    DSLValues ret_err = getDSL(B, ret, rt, DSLMap);
    B.CreateCall(rt.ShadowStackPush, {ret_err.xhat, ret_err.rhat, ret_err.sign, ret_err.ehat, ret_err.isExact, ret_err.relerr});
    return true;
}