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
    if (val->getType()->isDoubleTy()) {
        AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dsl.rhat, val, dsl.relerr});
    }
    else {
        AfterSI.CreateCall(rt.ShadowStoreF, {ptr, val, dsl.rhat, val, dsl.relerr});
    }
    return true;
}

bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, DSLValues> &DSLMap, 
                Value *sharedLoadOut) {
    if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
        return false;
    }
    IRBuilder<> AfterLI(LI->getNextNode());
    if (LI->isVolatile() || LI->isAtomic()) {
        DSLMap[LI] = getDSL(AfterLI, LI, rt, DSLMap);
        return true;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    // llvm::Value *outPtr = AfterLI.CreateAlloca(rt.ShadowEntryTy, nullptr, "load.out");
    if (LI->getType()->isDoubleTy()) {
        AfterLI.CreateCall(rt.ShadowLoadD, {ptr, LI, sharedLoadOut});
    }
    else {
        AfterLI.CreateCall(rt.ShadowLoadF, {ptr, LI, sharedLoadOut});
    }
    Value *entry = AfterLI.CreateLoad(rt.ShadowEntryTy, sharedLoadOut);
    DSLMap[LI] = extractDSL(AfterLI, entry);
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
    B.CreateCall(rt.ShadowStackPush, {ret_err.xhat, ret_err.rhat, ret_err.fpval, ret_err.relerr});
    return true;
}