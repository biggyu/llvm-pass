#include "ShadowMemory.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

static Value* getError(Value *v, Constant *ZeroF, Constant *ZeroD, 
                        DenseMap<const Value*, Value*> &ErrorMap) {
    auto it = ErrorMap.find(v);
    if (it != ErrorMap.end()) {
        return it->second;
    }
    if (v->getType()->isDoubleTy()) {
        return ZeroD;
    }
    if (v->getType()->isFloatTy()) {
        return ZeroF;
    }
    return nullptr;
}

bool handleStore(StoreInst *SI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap) {
    llvm::Value *val = SI->getValueOperand();
    if (!val->getType()->isDoubleTy() && !val->getType()->isFloatTy()) {
        return false;
    }
    if (SI->isVolatile() || SI->isAtomic()) {
        return false;
    }
    llvm::Value *ptr = SI->getPointerOperand();
    llvm::Value *dx = getError(val, rt.ZeroF, rt.ZeroD, ErrorMap);
    IRBuilder<> AfterSI(SI->getNextNode());
    if (val->getType()->isDoubleTy()) {
        AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dx});
    }
    else {
        AfterSI.CreateCall(rt.ShadowStoreF, {ptr, val, dx});
    }
    return true;
}

bool handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
        return false;
    }
    if (LI->isVolatile() || LI->isAtomic()) {
        return false;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    llvm::Value *dx;
    IRBuilder<> AfterLI(LI->getNextNode());
    dx = LI->getType()->isDoubleTy() ? AfterLI.CreateCall(rt.ShadowLoadD, {ptr}) : AfterLI.CreateCall(rt.ShadowLoadF, {ptr});
    // if (LI->getType()->isDoubleTy()) {
    //     dx = AfterLI.CreateCall(rt.ShadowLoadD, {ptr});
    // }
    // else {
    //     dx = AfterLI.CreateCall(rt.ShadowLoadF, {ptr});
    // }
    ErrorMap[LI] = dx;
    return true;
}