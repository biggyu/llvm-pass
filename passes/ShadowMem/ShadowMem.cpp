#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMem.h"
#include "runtime_decls.h"

using namespace llvm;

void runOnModule(llvm::Module &M) {
    utils::RuntimeFns rt(M);
    auto &Ctx = M.getContext();
    DenseMap<const Value*, Value*> errof;
    Type* StoreretTy = Type::getVoidTy(Ctx);
    Type* LoadretTy = Type::getDoubleTy(Ctx);
    
    // PointerType *PrintfArgTy = PointerType::getUnqual(Ctx);
    std::vector<Type*> StoreparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx)};
    std::vector<Type*> LoadparamTy = {PointerType::getUnqual(Ctx)};
    // std::vector<Type*> StoreparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx)};
    // PointerType *LoadparamTy = PointerType::getUnqual(Ctx);

    // FunctionType *PrintfTy = FunctionType::get(
    //     IntegerType::getInt32Ty(Ctx),
    //     PrintfArgTy,
    //     true);
    FunctionType *StoreTy = FunctionType::get(
        StoreretTy,
        StoreparamTy,
        false);
    FunctionType *LoadTy = FunctionType::get(
        LoadretTy,
        LoadparamTy,
        false);

    // FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
    FunctionCallee ShadowStore = M.getOrInsertFunction("shadow_store", StoreTy);
    FunctionCallee ShadowLoad = M.getOrInsertFunction("shadow_load", LoadTy);

    IRBuilder<> GlobalB(Ctx);

    // GlobalB.CreateGlobalStringPtr("()")
    for (Function &F: M) {
        if (F.isDeclaration()) continue;
        if (F.getName() == "shadow_store" || F.getName() == "shadow_load") continue;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isa<PHINode>(&I)) continue;
                IRBuilder<> Builder(&I);
                
                if (I.getOpcode() == Instruction::FAdd) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosum.x");
                        AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosum.dx");
                        Builder.CreateCall(rt.TwoSum, {opr0, opr1, x, dx});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, x, "twosum.val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, dx, "twosum.err");
                        errof[&I] = dxval;
                        
                        Value *fmt = Builder.CreateGlobalStringPtr("TwoSum: x=%f, dx=%e\n");
                        Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                else if (I.getOpcode() == Instruction::FSub) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosum.x");
                        AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosum.dx");
                        Value *invopr1 = Builder.CreateFNeg(opr1, "inv");
                        Builder.CreateCall(rt.TwoSum, {opr0, invopr1, x, dx});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, x, "twosum.val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, dx, "twosum.err");
                        errof[&I] = dxval;
                        
                        Value *fmt = Builder.CreateGlobalStringPtr("TwoSum: x=%f, dx=%e\n");
                        Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                else if (I.getOpcode() == Instruction::FMul) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprod.x");
                        AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprod.dx");
                        Builder.CreateCall(rt.TwoProd, {opr0, opr1, x, dx});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, x, "twoprod.val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, dx, "twoprod.err");
                        errof[&I] = dxval;

                        Value *fmt = Builder.CreateGlobalStringPtr("TwoProd: x=%f, dx=%e\n");
                        Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                if (auto *SI = dyn_cast<StoreInst>(&I)) {
                    llvm::Value *val = SI->getValueOperand();
                    llvm::Value *ptr = SI->getPointerOperand();
                    llvm::Value *dx = nullptr;
                    auto it = errof.find(val);
                    if (it != errof.end()) {
                        dx = it->second;
                    }
                    else {
                        dx = llvm::ConstantFP::get(llvm::Type::getDoubleTy(Ctx), 0.0);
                    }
                    if (val->getType()->isDoubleTy()) {
                        Builder.CreateCall(ShadowStore, {ptr, val, dx});
                    }
                    else if (val->getType()->isFloatTy()) {
                        llvm::Value *valD = Builder.CreateFPExt(val, llvm::Type::getDoubleTy(Ctx));
                        Builder.CreateCall(ShadowStore, {ptr, valD, dx});
                    }
                }
                else if (auto *LI = dyn_cast<LoadInst>(&I)) {
                    llvm::Value *ptr = LI->getPointerOperand();
                    llvm::Value *dx = Builder.CreateCall(ShadowLoad, {ptr});
                    errof[&I] = dx;
                }
            }
        }
    }
}

PreservedAnalyses ShadowMemPass::run(Module &M, ModuleAnalysisManager &) {
    runOnModule(M);
    return PreservedAnalyses::all();
    // bool Changed = runOnModule(M);
    // return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}
llvm::PassPluginLibraryInfo getShadowMemPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "ShadowMemPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "shadowmem") {
                            MPM.addPass(ShadowMemPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getShadowMemPluginInfo();
}