#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMem.h"
#include "runtime_decls.h"

using namespace llvm;

static Value* getError(Value *v, utils::RuntimeFns &rt, 
                        DenseMap<const Value*, Value*> &ErrorMap) {
// static Value* getError(Value *v, Constant *ZeroF, Constant *ZeroD, DenseMap<const Value*, Value*> &ErrorMap) {
    auto it = ErrorMap.find(v);
    if (it != ErrorMap.end()) {
        return it->second;
    }
    if (v->getType()->isDoubleTy()) {
        return rt.ZeroD;
        // return ZeroD;
    }
    if (v->getType()->isFloatTy()) {
        return rt.ZeroF;
        // return ZeroF;
    }
    return nullptr;
}

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
                // Constant *ZeroF, Constant *ZeroD,
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
        dx = val->getType()->isDoubleTy() ? (Value*)rt.ZeroD : (Value*)rt.ZeroF;
        // dx = val->getType()->isDoubleTy() ? (Value*)ZeroD : (Value*)ZeroF;
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
                // Constant *ZeroF, Constant *ZeroD,
                DenseMap<const Value*, Value*> &ErrorMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return;
    }
    Value *opr0 = BO->getOperand(0);
    Value *opr1 = BO->getOperand(1);
    Value *opr0_err = nullptr;
    Value *opr1_err = nullptr;
    opr0_err = getError(opr0, rt, ErrorMap);
    opr1_err = getError(opr1, rt, ErrorMap);
    // opr0_err = getError(opr0, ZeroF, ZeroD, ErrorMap);
    // opr1_err = getError(opr1, ZeroF, ZeroD, ErrorMap);
    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            if (BO->getType()->isDoubleTy()) {
                Builder.CreateCall(rt.PropSumDError, {opr0, opr0_err, opr1, opr1_err, valued, errord});
                // Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.PropSumFError, {opr0, opr0_err, opr1, opr1_err, valuef, errorf});
                // Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
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
                Builder.CreateCall(rt.PropSumDError, {opr0, opr0_err, invopr1, opr1_err, valued, errord});
                // Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twosum.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twosum.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.PropSumFError, {opr0, opr0_err, invopr1, opr1_err, valuef, errorf});
                // Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
                Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        }
        case Instruction::FMul:
            if (BO->getType()->isDoubleTy()) {
                Builder.CreateCall(rt.PropProdDError, {opr0, opr0_err, opr1, opr1_err, valued, errord});
                // Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "twoprod.double_val");
                Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "twoprod.double_err");
                ErrorMap[BO] = dxval;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Builder.CreateCall(rt.PropProdFError, {opr0, opr0_err, opr1, opr1_err, valuef, errorf});
                // Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twoprod.float_val");
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

    IRBuilder<> GlobalB(Ctx);

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        if (isRuntimeFunction(F)) continue;

        DenseMap<const Value*, Value*> ErrorMap;

        IRBuilder<> EntryBuilder(&F.getEntryBlock(), F.getEntryBlock().getFirstNonPHIOrDbgOrAlloca());
        AllocaInst *valuef = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "value.f");
        AllocaInst *errorf = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "error.f");
        AllocaInst *valued = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "value.d");
        AllocaInst *errord = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "error.d");

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
                handleStore(SI, Builder, rt, ErrorMap);
                // handleStore(SI, Builder, rt, ZeroF, ZeroD, ErrorMap);
                continue;
            }
            // if (auto *II = dyn_cast<IntrinsicInst>(I)) {
            //     if(handleIntrinsic(II)) {
            //         continue;
            //     }
            // }
            if (auto *BI = dyn_cast<BinaryOperator>(I)) {
                handleBinary(BI, Builder, rt, valuef, errorf, valued, errord, ErrorMap);
                // handleBinary(BI, Builder, rt, valuef, errorf, valued, errord, ZeroF, ZeroD, ErrorMap);
            }
        }
    }
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