#include "ShadowMemory.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

static Value* getError(Value *v, Constant *ZeroD, 
                        DenseMap<const Value*, Value*> &ErrorMap) {
    auto it = ErrorMap.find(v);
    if (it != ErrorMap.end()) {
        return it->second;
    }
    // if (!isa<ConstantFP>(v)) {
    //     llvm::errs() << "ShadowMemory getError MISS on: ";
    //     v->print(llvm::errs());
    //     llvm::errs() << "\n";
    // }
    return ZeroD;
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
    llvm::Value *dx = getError(val, rt.ZeroD, ErrorMap);
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
        ErrorMap[LI] = rt.ZeroD;
        return false;
    }
    llvm::Value *dx, *ptr = LI->getPointerOperand();
    IRBuilder<> AfterLI(LI->getNextNode());
    dx = LI->getType()->isDoubleTy() ? AfterLI.CreateCall(rt.ShadowLoadD, {ptr, LI}) : AfterLI.CreateCall(rt.ShadowLoadF, {ptr, LI});
    // if (LI->getType()->isDoubleTy()) {
    //     dx = AfterLI.CreateCall(rt.ShadowLoadD, {ptr});
    // }
    // else {
    //     dx = AfterLI.CreateCall(rt.ShadowLoadF, {ptr});
    // }
    ErrorMap[LI] = dx;
    return true;
}

bool handleReturn(ReturnInst *RI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap) {
    Value *ret = RI->getReturnValue();
    if (!ret || (!ret->getType()->isDoubleTy() && !ret->getType()->isFloatTy())) {
        return false;
    }
    IRBuilder<> B(RI);
    Value *ret_err = getError(ret, rt.ZeroD, ErrorMap);
    B.CreateCall(rt.ShadowStackPush, {ret_err});
    return true;
}