#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMem.h"
#include "runtime_decls.h"

using namespace llvm;

static bool isRuntimeFunction(const Function &F) {
    StringRef N = F.getName();
    return N == "shadow_store_double" ||
           N == "shadow_load_double"  ||
           N == "shadow_store_float"  ||
           N == "shadow_load_float"   ||
           N == "TwoSumD" || N == "TwoSumF" ||
           N == "TwoProdD" || N == "TwoProdF";
}

void handleStore(StoreInst *SI, IRBuilder<> &Builder, utils::RuntimeFns &rt,
                Constant *ZeroF, Constant *ZeroD,
                DenseMap<const Value*, Value*> &ErrorMap) {
    llvm::Value *val = SI->getValueOperand();
    if (!val->getType()->isDoubleTy() && !val->getType()->isFloatTy()) {
        return;
    }
    llvm::Value *ptr = SI->getPointerOperand();
    llvm::Value *dx = nullptr;
    auto it = ErrorMap.find(val);
    if (it != ErrorMap.end()) {
        dx = it->second;
    }
    else {
        dx = val->getType()->isDoubleTy() ? (Value*)ZeroD : (Value*)ZeroF;
    }
    if (val->getType()->isDoubleTy()) {
        Builder.CreateCall(rt.ShadowStoreD, {ptr, val, dx});
    }
    else {
        Builder.CreateCall(rt.ShadowStoreF, {ptr, val, dx});
    }
}

void handleLoad(LoadInst *LI, IRBuilder<> &Builder, utils::RuntimeFns &rt, 
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
        return;
    }
    llvm::Value *ptr = LI->getPointerOperand();
    llvm::Value *dx;
    if (LI->getType()->isDoubleTy()) {
        dx = Builder.CreateCall(rt.ShadowLoadD, {ptr});
    }
    else {
        dx = Builder.CreateCall(rt.ShadowLoadF, {ptr});
    }
    ErrorMap[LI] = dx;

    // llvm::Value *dx = Builder.CreateCall(ShadowLoad, {ptr});
    // ErrorMap[&I] = dx;
}

void handleBinary(Instruction *BO, IRBuilder<> &Builder, utils::RuntimeFns &rt,
                AllocaInst *valuef, AllocaInst *errorf, 
                AllocaInst *valued, AllocaInst *errord,
                DenseMap<const Value*, Value*> &ErrorMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return;
    }
    Value *opr0 = BO->getOperand(0);
    Value *opr1 = BO->getOperand(1);
    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            if (BO->getType()->isDoubleTy()) {
                Builder.CreateCall(rt.TwoSumD, {opr0, opr1, valued, errord});
                Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.TwoSumF, {opr0, opr1, valuef, errorf});
                Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
                Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        }
        case Instruction::FSub: {
            Value *invopr1 = Builder.CreateFNeg(opr1, "inv");
            if (BO->getType()->isDoubleTy()) {
                Builder.CreateCall(rt.TwoSumD, {opr0, invopr1, valued, errord});
                Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.TwoSumF, {opr0, invopr1, valuef, errorf});
                Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
                Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        }
        case Instruction::FMul:
            if (BO->getType()->isDoubleTy()) {
                Builder.CreateCall(rt.TwoProdD, {opr0, opr1, valued, errord});
                Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twoprod.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twoprod.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.TwoProdF, {opr0, opr1, valuef, errorf});
                Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twoprod.float_val");
                Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twoprod.float_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        default:
            break;
    }
}

void runOnModule(llvm::Module &M) {
    utils::RuntimeFns rt(M);
    auto &Ctx = M.getContext();
    // Type* StoreretTy = Type::getVoidTy(Ctx);
    // Type* LoadDretTy = Type::getDoubleTy(Ctx);
    // Type* LoadFretTy = Type::getFloatTy(Ctx);
    
    // PointerType *PrintfArgTy = PointerType::getUnqual(Ctx);
    // std::vector<Type*> StoreDparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx)};
    // std::vector<Type*> LoadDparamTy = {PointerType::getUnqual(Ctx)};
    // std::vector<Type*> StoreFparamTy = {PointerType::getUnqual(Ctx), Type::getFloatTy(Ctx), Type::getFloatTy(Ctx)};
    // std::vector<Type*> LoadFparamTy = {PointerType::getUnqual(Ctx)};
    // std::vector<Type*> StoreparamTy = {PointerType::getUnqual(Ctx), Type::getDoubleTy(Ctx)};
    // PointerType *LoadparamTy = PointerType::getUnqual(Ctx);

    // FunctionType *PrintfTy = FunctionType::get(
    //     IntegerType::getInt32Ty(Ctx),
    //     PrintfArgTy,
    //     true);
    // FunctionType *StoreDTy = FunctionType::get(
    //     StoreretTy,
    //     StoreDparamTy,
    //     false);
    // FunctionType *LoadDTy = FunctionType::get(
    //     LoadDretTy,
    //     LoadDparamTy,
    //     false);
    // FunctionType *StoreFTy = FunctionType::get(
    //     StoreretTy,
    //     StoreFparamTy,
    //     false);
    // FunctionType *LoadFTy = FunctionType::get(
    //     LoadFretTy,
    //     LoadFparamTy,
    //     false);

    // FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
    // FunctionCallee ShadowStoreD = M.getOrInsertFunction("shadow_store_double", StoreDTy);
    // FunctionCallee ShadowLoadD = M.getOrInsertFunction("shadow_load_double", LoadDTy);
    // FunctionCallee ShadowStoreF = M.getOrInsertFunction("shadow_store_float", StoreFTy);
    // FunctionCallee ShadowLoadF = M.getOrInsertFunction("shadow_load_float", LoadFTy);

    IRBuilder<> GlobalB(Ctx);

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        if (isRuntimeFunction(F)) continue;

        DenseMap<const Value*, Value*> ErrorMap;
        // DenseMap<const Value*, Value*> ErrorMap;

        IRBuilder<> EntryBuilder(&F.getEntryBlock(), F.getEntryBlock().getFirstNonPHIOrDbgOrAlloca());
        AllocaInst *valuef = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "value.f");
        AllocaInst *errorf = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "error.f");
        AllocaInst *valued = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "value.d");
        AllocaInst *errord = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "error.d");

        Constant *ZeroD = ConstantFP::get(rt.DoubleTy, 0.0);
        Constant *ZeroF = ConstantFP::get(rt.FloatTy, 0.0);

        SmallVector<Instruction*, 128> WorkList;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                WorkList.push_back(&I);
            }
        }
        for (auto *I : WorkList) {
            if (isa<PHINode>(I)) continue;

            IRBuilder<> Builder(I);

            if (auto *LI = dyn_cast<LoadInst>(I)) {
                handleLoad(LI, Builder, rt, ErrorMap);
                continue;
            }
            if (auto *SI = dyn_cast<StoreInst>(I)) {
                handleStore(SI, Builder, rt, ZeroF, ZeroD, ErrorMap);
                continue;
            }
            // if (auto *II = dyn_cast<IntrinsicInst>(I)) {
            //     if(handleIntrinsic(II)) {
            //         continue;
            //     }
            // }
            if (auto *BI = dyn_cast<BinaryOperator>(I)) {
                handleBinary(BI, Builder, rt, valuef, errorf, valued, errord, ErrorMap);
            }
        }
    }



    // for (Function &F: M) {
    //     if (F.isDeclaration()) continue;
    //     if (isRuntimeFunction(F)) continue;
        
    //     DenseMap<const Value*, Value*> ErrorMap;
    //     IRBuilder<> EntryBuilder(&F.getEntryBlock(), F.getEntryBlock().getFirstNonPHIOrDbgOrAlloca());

    //     //Store values, errors of twosum, twoprods
    //     AllocaInst *valuef = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "value.f");
    //     AllocaInst *errorf = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "error.f");
    //     AllocaInst *valued = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "value.d");
    //     AllocaInst *errord = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "error.d");
        
    //     for (BasicBlock &BB : F) {
    //         for (Instruction &I : BB) {
    //             if (isa<PHINode>(&I)) continue;
    //             IRBuilder<> Builder(&I);
                
    //             //! Implement for fmuladd
    //             // if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
    //             //     if (II->getIntrinsicID() == Intrinsic::fmuladd) {
    //             //         // llvm::errs() << "fmuladd\n";
    //             //         Value *a = II->getArgOperand(0);
    //             //         Value *b = II->getArgOperand(1);
    //             //         Value *c = II->getArgOperand(2);
    //             //         if (II->getArgOperand(0)->getType()->isDoubleTy()) {
    //             //             Builder.CreateCall(rt.TwoProdD, {a, b, valued, errord});
    //             //             Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
    //             //             // Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
    //             //             // errof[&I] = dxval;
    //             //             Builder.CreateCall(rt.TwoSumD, {xval, c, valued, errord});
    //             //             Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twoprod.double_val");
    //             //             Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twoprod.double_err");
    //             //             errof[&I] = dxval;
                            
    //             //         }
    //             //         else if (II->getArgOperand(0)->getType()->isFloatTy()) {
    //             //             Builder.CreateCall(rt.TwoProdF, {a, b, valuef, errorf});
    //             //             Value *xval = Builder.CreateLoad(rt.FloatTy, valued, "twosum.float_val");
    //             //             // Value *dxval = Builder.CreateLoad(rt.FloatTy, errord, "twosum.float_err");
    //             //             // errof[&I] = dxval;
    //             //             Builder.CreateCall(rt.TwoSumF, {xval, c, valuef, errorf});
    //             //             Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twoprod.float_val");
    //             //             Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twoprod.float_err");
    //             //             errof[&I] = dxval;
    //             //         }
    //             //     }
    //             // }
    //             if (I.getOpcode() == Instruction::FAdd) {
    //                 if (I.getOperand(0)->getType()->isDoubleTy()) {
    //                     Value *opr0 = I.getOperand(0);
    //                     Value *opr1 = I.getOperand(1);
    //                     // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.x");
    //                     // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.dx");
                        
    //                     Builder.CreateCall(rt.TwoSumD, {opr0, opr1, valued, errord});
    //                     Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
    //                     Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
    //                     ErrorMap[&I] = dxval;

    //                     // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumD: x=%f, dx=%e\n");
    //                     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    //                 }
    //                 else if (I.getOperand(0)->getType()->isFloatTy()) {
    //                     Value *opr0 = I.getOperand(0);
    //                     Value *opr1 = I.getOperand(1);
    //                     // AllocaInst *x = Builder.CreateAlloca(rt.FloatTy, nullptr, "twosumf.x");
    //                     // AllocaInst *dx = Builder.CreateAlloca(rt.FloatTy, nullptr, "twosumf.dx");
    //                     Builder.CreateCall(rt.TwoSumF, {opr0, opr1, valuef, errorf});
    //                     Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
    //                     Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
    //                     ErrorMap[&I] = dxval;
                        
    //                     // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumF: x=%f, dx=%e\n");
    //                     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    //                 }
    //             }
    //             else if (I.getOpcode() == Instruction::FSub) {
    //                 if (I.getOperand(0)->getType()->isDoubleTy()) {
    //                     Value *opr0 = I.getOperand(0);
    //                     Value *opr1 = I.getOperand(1);
    //                     // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.x");
    //                     // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twosumd.dx");
    //                     Value *invopr1 = Builder.CreateFNeg(opr1, "inv");
    //                     Builder.CreateCall(rt.TwoSumD, {opr0, invopr1, valued, errord});
    //                     Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
    //                     Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
    //                     ErrorMap[&I] = dxval;
                        
    //                     // Value *fmt = Builder.CreateGlobalStringPtr("TwoSum: x=%f, dx=%e\n");
    //                     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    //                 }
    //             }
    //             else if (I.getOpcode() == Instruction::FMul) {
    //                 if (I.getOperand(0)->getType()->isDoubleTy()) {
    //                     Value *opr0 = I.getOperand(0);
    //                     Value *opr1 = I.getOperand(1);
    //                     // AllocaInst *x = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprodd.x");
    //                     // AllocaInst *dx = Builder.CreateAlloca(rt.DoubleTy, nullptr, "twoprodd.dx");
    //                     Builder.CreateCall(rt.TwoProdD, {opr0, opr1, valued, errord});
    //                     Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twoprod.double_val");
    //                     Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twoprod.double_err");
    //                     ErrorMap[&I] = dxval;

    //                     // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdf: x=%f, dx=%e\n");
    //                     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    //                 }
    //                 else if (I.getOperand(0)->getType()->isFloatTy()) {
    //                     Value *opr0 = I.getOperand(0);
    //                     Value *opr1 = I.getOperand(1);
    //                     // AllocaInst *x = Builder.CreateAlloca(rt.FloatTy, nullptr, "twoprodf.x");
    //                     // AllocaInst *dx = Builder.CreateAlloca(rt.FloatTy, nullptr, "twoprodf.dx");
    //                     Builder.CreateCall(rt.TwoProdF, {opr0, opr1, valuef, errorf});
    //                     Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twoprod.float_val");
    //                     Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twoprod.float_err");
    //                     ErrorMap[&I] = dxval;

    //                     // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdf: x=%f, dx=%e\n");
    //                     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    //                 }
    //             }
    //             if (auto *SI = dyn_cast<StoreInst>(&I)) {
    //                 llvm::Value *val = SI->getValueOperand();
    //                 // llvm::Value *ptr = SI->getPointerOperand();
    //                 // llvm::Value *dx = nullptr;
    //                 // auto it = ErrorMap.find(val);
    //                 // if (it != ErrorMap.end()) {
    //                 //     dx = it->second;
    //                 // }
    //                 // else {
    //                 //     dx = llvm::ConstantFP::get(llvm::Type::getDoubleTy(Ctx), 0.0);
    //                 // }
    //                 // if (val->getType()->isDoubleTy()) {
    //                 //     Builder.CreateCall(ShadowStore, {ptr, val, dx});
    //                 // }
    //                 // else if (val->getType()->isFloatTy()) {
    //                 //     llvm::Value *valD = Builder.CreateFPExt(val, llvm::Type::getDoubleTy(Ctx));
    //                 //     Builder.CreateCall(ShadowStore, {ptr, valD, dx});
    //                 // }
    //                 if (val->getType()->isDoubleTy()) {
    //                     llvm::Value *ptr = SI->getPointerOperand();
    //                     llvm::Value *dx = nullptr;
    //                     auto it = ErrorMap.find(val);
    //                     if (it != ErrorMap.end()) {
    //                         dx = it->second;
    //                     }
    //                     else {
    //                         dx = llvm::ConstantFP::get(llvm::Type::getDoubleTy(Ctx), 0.0);
    //                     }
    //                     Builder.CreateCall(ShadowStoreD, {ptr, val, dx});
    //                 }
    //                 else if (val->getType()->isFloatTy()) {
    //                     llvm::Value *ptr = SI->getPointerOperand();
    //                     llvm::Value *dx = nullptr;
    //                     auto it = ErrorMap.find(val);
    //                     if (it != ErrorMap.end()) {
    //                         dx = it->second;
    //                     }
    //                     else {
    //                         dx = llvm::ConstantFP::get(llvm::Type::getFloatTy(Ctx), 0.0);
    //                     }
    //                     Builder.CreateCall(ShadowStoreF, {ptr, val, dx});
    //                 }
    //             }
    //             else if (auto *LI = dyn_cast<LoadInst>(&I)) {
    //                 llvm::Value *ptr = LI->getPointerOperand();
    //                 if (LI->getType()->isDoubleTy()) {
    //                     llvm::Value *dx = Builder.CreateCall(ShadowLoadD, {ptr});
    //                     ErrorMap[&I] = dx;
    //                 }
    //                 else if (LI->getType()->isFloatTy()) {
    //                     llvm::Value *dx = Builder.CreateCall(ShadowLoadF, {ptr});
    //                     ErrorMap[&I] = dx;
    //                 }
                    
    //                 // llvm::Value *dx = Builder.CreateCall(ShadowLoad, {ptr});
    //                 // ErrorMap[&I] = dx;
    //             }
    //         }
    //     }
    // }
}

PreservedAnalyses ShadowMemPass::run(Module &M, ModuleAnalysisManager &) {
    runOnModule(M);
    return PreservedAnalyses::none();
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