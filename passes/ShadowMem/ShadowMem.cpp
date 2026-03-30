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
    Type* LoadDretTy = Type::getDoubleTy(Ctx);
    Type* LoadFretTy = Type::getDoubleTy(Ctx);
    
    // PointerType *PrintfArgTy = PointerType::getUnqual(Ctx);
    std::vector<Type*> StoreDparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx)};
    std::vector<Type*> LoadDparamTy = {PointerType::getUnqual(Ctx)};
    std::vector<Type*> StoreFparamTy = {PointerType::getUnqual(Ctx), Type::getFloatTy(Ctx), Type::getFloatTy(Ctx)};
    std::vector<Type*> LoadFparamTy = {PointerType::getUnqual(Ctx)};
    // std::vector<Type*> StoreparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx)};
    // PointerType *LoadparamTy = PointerType::getUnqual(Ctx);

    // FunctionType *PrintfTy = FunctionType::get(
    //     IntegerType::getInt32Ty(Ctx),
    //     PrintfArgTy,
    //     true);
    FunctionType *StoreDTy = FunctionType::get(
        StoreretTy,
        StoreDparamTy,
        false);
    FunctionType *LoadDTy = FunctionType::get(
        LoadDretTy,
        LoadDparamTy,
        false);
    FunctionType *StoreFTy = FunctionType::get(
        StoreretTy,
        StoreFparamTy,
        false);
    FunctionType *LoadFTy = FunctionType::get(
        LoadFretTy,
        LoadFparamTy,
        false);

    // FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
    FunctionCallee ShadowStoreD = M.getOrInsertFunction("shadow_store_double", StoreDTy);
    FunctionCallee ShadowLoadD = M.getOrInsertFunction("shadow_load_double", LoadDTy);
    FunctionCallee ShadowStoreF = M.getOrInsertFunction("shadow_store_float", StoreFTy);
    FunctionCallee ShadowLoadF = M.getOrInsertFunction("shadow_load_float", LoadFTy);

    IRBuilder<> GlobalB(Ctx);

    // GlobalB.CreateGlobalStringPtr("()")
    for (Function &F: M) {
        if (F.isDeclaration()) continue;
        // BasicBlock EntryBlock = F.getEntryBlock();
        IRBuilder<> EntryBuilder(&F.getEntryBlock(), F.getEntryBlock().getFirstNonPHIOrDbgOrAlloca());

        //Store values, errors of twosum, twoprods
        AllocaInst *valuef = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "value.f");
        AllocaInst *errorf = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "error.f");
        AllocaInst *valued = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "value.d");
        AllocaInst *errord = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "error.d");

        if (F.getName() == "shadow_store" || F.getName() == "shadow_load") continue;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (isa<PHINode>(&I)) continue;
                IRBuilder<> Builder(&I);
                
                //! Implement for fmuladd
                if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
                    if (II->getIntrinsicID() == Intrinsic::fmuladd) {
                        // llvm::errs() << "fmuladd\n";
                        Value *a = II->getArgOperand(0);
                        Value *b = II->getArgOperand(1);
                        Value *c = II->getArgOperand(2);
                        if (II->getArgOperand(0)->getType()->isDoubleTy()) {
                            llvm::errs() << "double\n";
                        }
                        if (II->getArgOperand(0)->getType()->isFloatTy()) {
                            llvm::errs() << "Float\n";
                        }
                    }
                }
                if (I.getOpcode() == Instruction::FAdd) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.x");
                        // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.dx");
                        
                        Builder.CreateCall(rt.TwoSumD, {opr0, opr1, valued, errord});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                        errof[&I] = dxval;

                        // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumD: x=%f, dx=%e\n");
                        // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                    else if (I.getOperand(0)->getType()->isFloatTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        // AllocaInst *x = Builder.CreateAlloca(rt.FloatTy, nullptr, "twosumf.x");
                        // AllocaInst *dx = Builder.CreateAlloca(rt.FloatTy, nullptr, "twosumf.dx");
                        Builder.CreateCall(rt.TwoSumF, {opr0, opr1, valuef, errorf});
                        Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
                        Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
                        errof[&I] = dxval;
                        
                        // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumF: x=%f, dx=%e\n");
                        // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                else if (I.getOpcode() == Instruction::FSub) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.x");
                        // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.dx");
                        Value *invopr1 = Builder.CreateFNeg(opr1, "inv");
                        Builder.CreateCall(rt.TwoSumD, {opr0, invopr1, valued, errord});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                        errof[&I] = dxval;
                        
                        // Value *fmt = Builder.CreateGlobalStringPtr("TwoSum: x=%f, dx=%e\n");
                        // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                else if (I.getOpcode() == Instruction::FMul) {
                    if (I.getOperand(0)->getType()->isDoubleTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprodd.x");
                        // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprodd.dx");
                        Builder.CreateCall(rt.TwoProdD, {opr0, opr1, valued, errord});
                        Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twoprod.double_val");
                        Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twoprod.double_err");
                        errof[&I] = dxval;

                        // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdf: x=%f, dx=%e\n");
                        // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                    else if (I.getOperand(0)->getType()->isFloatTy()) {
                        Value *opr0 = I.getOperand(0);
                        Value *opr1 = I.getOperand(1);
                        // AllocaInst *x = Builder.CreateAlloca(rt.FloatTy, nullptr, "twoprodf.x");
                        // AllocaInst *dx = Builder.CreateAlloca(rt.FloatTy, nullptr, "twoprodf.dx");
                        Builder.CreateCall(rt.TwoProdF, {opr0, opr1, valuef, errorf});
                        Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twoprod.float_val");
                        Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twoprod.float_err");
                        errof[&I] = dxval;

                        // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdf: x=%f, dx=%e\n");
                        // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
                    }
                }
                if (auto *SI = dyn_cast<StoreInst>(&I)) {
                    llvm::Value *val = SI->getValueOperand();
                    // llvm::Value *ptr = SI->getPointerOperand();
                    // llvm::Value *dx = nullptr;
                    // auto it = errof.find(val);
                    // if (it != errof.end()) {
                    //     dx = it->second;
                    // }
                    // else {
                    //     dx = llvm::ConstantFP::get(llvm::Type::getDoubleTy(Ctx), 0.0);
                    // }
                    // if (val->getType()->isDoubleTy()) {
                    //     Builder.CreateCall(ShadowStore, {ptr, val, dx});
                    // }
                    // else if (val->getType()->isFloatTy()) {
                    //     llvm::Value *valD = Builder.CreateFPExt(val, llvm::Type::getDoubleTy(Ctx));
                    //     Builder.CreateCall(ShadowStore, {ptr, valD, dx});
                    // }
                    if (val->getType()->isDoubleTy()) {
                        llvm::Value *ptr = SI->getPointerOperand();
                        llvm::Value *dx = nullptr;
                        auto it = errof.find(val);
                        if (it != errof.end()) {
                            dx = it->second;
                        }
                        else {
                            dx = llvm::ConstantFP::get(llvm::Type::getDoubleTy(Ctx), 0.0);
                        }
                        Builder.CreateCall(ShadowStoreD, {ptr, val, dx});
                    }
                    else if (val->getType()->isFloatTy()) {
                        llvm::Value *ptr = SI->getPointerOperand();
                        llvm::Value *dx = nullptr;
                        auto it = errof.find(val);
                        if (it != errof.end()) {
                            dx = it->second;
                        }
                        else {
                            dx = llvm::ConstantFP::get(llvm::Type::getFloatTy(Ctx), 0.0);
                        }
                        Builder.CreateCall(ShadowStoreF, {ptr, val, dx});
                    }
                }
                else if (auto *LI = dyn_cast<LoadInst>(&I)) {
                    llvm::Value *ptr = LI->getPointerOperand();
                    if (LI->getType()->isDoubleTy()) {
                        llvm::Value *dx = Builder.CreateCall(ShadowLoadD, {ptr});
                        errof[&I] = dx;
                    }
                    else if (LI->getType()->isFloatTy()) {
                        llvm::Value *dx = Builder.CreateCall(ShadowLoadF, {ptr});
                        errof[&I] = dx;
                    }
                    
                    // llvm::Value *dx = Builder.CreateCall(ShadowLoad, {ptr});
                    // errof[&I] = dx;
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