#include "ShadowMemory.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

//! Move to getDSL()
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
        //TODO: Generate dummy DSLValue?
        // DSLMap[LI] = rt.ZeroD;
        return false;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    IRBuilder<> AfterLI(LI->getNextNode());
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

//TODO: Modify to DSLValue
bool handleReturn(ReturnInst *RI, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap) {
    Value *ret = RI->getReturnValue();
    if (!ret || (!ret->getType()->isDoubleTy() && !ret->getType()->isFloatTy())) {
        return false;
    }
    IRBuilder<> B(RI);
    //! Move to getDSL
    Value *ret_err = getError(ret, rt.ZeroD, ErrorMap);
    B.CreateCall(rt.ShadowStackPush, {ret_err});
    return true;
}