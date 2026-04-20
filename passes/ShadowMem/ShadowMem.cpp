#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMem.h"
#include "decls_fp.h"
#include "decls_mpfr.h"

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
           N == "PropSumDError" || N == "PropSumFError" ||
           N == "PropProdDError" || N == "PropProdFError" ||
           N == "PropDivDError" || N == "PropDivFError" ||
           N == "PropSqrtDError" || N == "PropProdFError";
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

bool handleIntrinsic(IntrinsicInst *II, IRBuilder<> &Builder, utils::RuntimeFns &rt, utils::RuntimeMPFRFns &rt_mpfr,
                // AllocaInst *valuef, AllocaInst *errorf, 
                // AllocaInst *valued, AllocaInst *errord,
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (II->getIntrinsicID() == Intrinsic::sqrt) {
        Value *arg0 = II->getArgOperand(0);
        Value *arg0_err = getError(arg0, rt, ErrorMap);
        if(II->getType()->isDoubleTy()) {
            Value *ret = Builder.CreateCall(rt.PropSqrtDError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "sqrt.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "sqrt.err");
            
            ErrorMap[II] = dx;
            // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
            // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
        }
        else if(II->getType()->isFloatTy()) {
            Value *ret = Builder.CreateCall(rt.PropSqrtFError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "sqrtf.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "sqrtf.err");
            
            ErrorMap[II] = dx;
            // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
            // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
        }
        return true;
    }
    else if (II->getIntrinsicID() == Intrinsic::fabs) {
        Value *arg0 = II->getArgOperand(0);
        Value *arg0_err = getError(arg0, rt, ErrorMap);
        if (II->getType()->isDoubleTy()) {
            Value *ret = Builder.CreateCall(rt_mpfr.PropFabsDError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "fabs.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "fabs.err");

            ErrorMap[II] = dx;
        }
        else if (II->getType()->isFloatTy()) {
            Value *ret = Builder.CreateCall(rt_mpfr.PropFabsFError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "fabsf.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "fabsf.err");

            ErrorMap[II] = dx;
        }
        return true;
    }
    else {
        return false;
    }
}
bool handleExternal(CallInst *CI, IRBuilder<> &Builder, utils::RuntimeFns &rt, utils::RuntimeMPFRFns &rt_mpfr,
                // AllocaInst *valuef, AllocaInst *errorf, 
                // AllocaInst *valued, AllocaInst *errord,
                DenseMap<const Value*, Value*> &ErrorMap) {
    if (Function *Callee = CI->getCalledFunction()) {
        StringRef N = Callee->getName();
        if (N == "sqrtf") {
            Value *arg0 = CI->getArgOperand(0);
            Value *arg0_err = getError(arg0, rt, ErrorMap);
            Value *ret = Builder.CreateCall(rt.PropSqrtFError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "sqrtf.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "sqrtf.err");
            
            ErrorMap[CI] = dx;
            // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
            // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            // return true;
        }
        else if (N == "sqrt") {
            Value *arg0 = CI->getArgOperand(0);
            Value *arg0_err = getError(arg0, rt, ErrorMap);
            Value *ret = Builder.CreateCall(rt.PropSqrtDError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "sqrt.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "sqrt.err");
            
            ErrorMap[CI] = dx;
            // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
            // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            // return true;
        }
        else if (N == "expf") {
            Value *arg0 = CI->getArgOperand(0);
            Value *arg0_err = getError(arg0, rt, ErrorMap);
            Value *ret = Builder.CreateCall(rt_mpfr.PropExpFError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "expf.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "expf.err");
            ErrorMap[CI] = dx;
            // return true;
        }
        else if (N == "exp") {
            Value *arg0 = CI->getArgOperand(0);
            Value *arg0_err = getError(arg0, rt, ErrorMap);
            Value *ret = Builder.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
            // Value *x = Builder.CreateExtractValue(ret, {0}, "exp.val");
            Value *dx = Builder.CreateExtractValue(ret, {1}, "exp.err");
            ErrorMap[CI] = dx;
            // return true;
        }
        else {
            return false;
        }
        return true;
        // if (N != "sqrt" && N != "sqrtf" && N != "fabs" && N != "exp") {
        //     return false;
        // }
    }
    return false;
    // Value *arg0 = CI->getArgOperand(0);
    // Value *arg0_err = getError(arg0, rt, ErrorMap);
    
    // if(CI->getType()->isDoubleTy()) {
    //     Builder.CreateCall(rt.PropSqrtDError, {arg0, arg0_err, valued, errord});
    //     Value *xval = Builder.CreateLoad(rt.DoubleTy, valued, "sqrt.double_val");
    //     Value *dxval = Builder.CreateLoad(rt.DoubleTy, errord, "sqrt.double_err");
    //     ErrorMap[CI] = dxval;
    //     // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
    //     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    // }
    // else if(CI->getType()->isFloatTy()) {
    //     Builder.CreateCall(rt.PropSqrtFError, {arg0, arg0_err, valuef, errorf});
    //     Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "sqrt.float_val");
    //     Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "sqrt.float_err");
    //     ErrorMap[CI] = dxval;
    //     // Value *fmt = Builder.CreateGlobalStringPtr("sqrtD: x=%f, dx=%e\n");
    //     // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
    // }
}

void handleBinary(Instruction *BO, IRBuilder<> &Builder, utils::RuntimeFns &rt,
                // AllocaInst *valuef, AllocaInst *errorf, 
                // AllocaInst *valued, AllocaInst *errord,
                // Constant *ZeroF, Constant *ZeroD,
                DenseMap<const Value*, Value*> &ErrorMap) {
    
    if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
        return;
    }
    Value *opr0 = BO->getOperand(0);
    Value *opr1 = BO->getOperand(1);
    Value *opr0_err = getError(opr0, rt, ErrorMap);
    Value *opr1_err = getError(opr1, rt, ErrorMap);
    // opr0_err = getError(opr0, rt, ErrorMap);
    // opr1_err = getError(opr1, rt, ErrorMap);
    // opr0_err = getError(opr0, ZeroF, ZeroD, ErrorMap);
    // opr1_err = getError(opr1, ZeroF, ZeroD, ErrorMap);
    switch (BO->getOpcode()) {
        case Instruction::FAdd: {
            if (BO->getType()->isDoubleTy()) {
                Value *ret = Builder.CreateCall(rt.PropSumDError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "addd.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "addd.err");
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Value *ret = Builder.CreateCall(rt.PropSumFError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "addf.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "addf.err");
                // Value *xval = Builder.CreateLoad(rt.FloatTy, valuef, "twosum.float_val");
                // Value *dxval = Builder.CreateLoad(rt.FloatTy, errorf, "twosum.float_err");
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoSumF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        }
        case Instruction::FSub: {
            Value *invopr1 = Builder.CreateFNeg(opr1, "inv.val");
            Value *invopr1_err = Builder.CreateFNeg(opr1_err, "inv.err");
            if (BO->getType()->isDoubleTy()) {
                Value *ret = Builder.CreateCall(rt.PropSumDError, {opr0, opr0_err, invopr1, invopr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "subd.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "subd.err");
                
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Value *ret = Builder.CreateCall(rt.PropSumFError, {opr0, opr0_err, invopr1, invopr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "subf.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "subf.err");
                
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("InvTwoSumF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        }
        case Instruction::FMul:
            if (BO->getType()->isDoubleTy()) {
                Value *ret = Builder.CreateCall(rt.PropProdDError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "muld.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "muld.err");
                
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Value *ret = Builder.CreateCall(rt.PropProdFError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "mulf.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "mulf.err");
                
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdF: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            break;
        case Instruction::FDiv:
            if (BO->getType()->isDoubleTy()) {
                Value *ret = Builder.CreateCall(rt.PropDivDError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "divd.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "divd.err");
                
                ErrorMap[BO] = dx;
                // Value *fmt = Builder.CreateGlobalStringPtr("TwoProdD: x=%f, dx=%e\n");
                // Builder.CreateCall(rt.Printf, {fmt, xval, dxval});
            }
            else {
                Value *ret = Builder.CreateCall(rt.PropDivFError, {opr0, opr0_err, opr1, opr1_err});
                // Value *x = Builder.CreateExtractValue(ret, {0}, "divf.val");
                Value *dx = Builder.CreateExtractValue(ret, {1}, "divf.err");
                
                ErrorMap[BO] = dx;
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
    utils::RuntimeMPFRFns rt_mpfr(M);
    auto &Ctx = M.getContext();

    IRBuilder<> GlobalB(Ctx);

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        if (isRuntimeFunction(F)) continue;

        DenseMap<const Value*, Value*> ErrorMap;

        IRBuilder<> EntryBuilder(&F.getEntryBlock(), F.getEntryBlock().getFirstNonPHIOrDbgOrAlloca());
        // AllocaInst *valuef = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "value.f");
        // AllocaInst *errorf = EntryBuilder.CreateAlloca(rt.FloatTy, nullptr, "error.f");
        // AllocaInst *valued = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "value.d");
        // AllocaInst *errord = EntryBuilder.CreateAlloca(rt.DoubleTy, nullptr, "error.d");

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
            if (auto *II = dyn_cast<IntrinsicInst>(I)) {
                if(handleIntrinsic(II, Builder, rt, rt_mpfr, ErrorMap)) {
                // if(handleIntrinsic(II, Builder, rt, rt_mpfr, valuef, errorf, valued, errord, ErrorMap)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<CallInst>(I)) {
                if(handleExternal(CI, Builder, rt, rt_mpfr, ErrorMap)) {
                // if(handleExternal(CI, Builder, rt, rt_mpfr, valuef, errorf, valued, errord, ErrorMap)) {
                    continue;
                }
            }
            if (auto *BI = dyn_cast<BinaryOperator>(I)) {
                handleBinary(BI, Builder, rt, ErrorMap);
                // handleBinary(BI, Builder, rt, valuef, errorf, valued, errord, ErrorMap);
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