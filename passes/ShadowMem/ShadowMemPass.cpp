#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMemPass.h"
#include "ErrorProp.h"
#include "ShadowMemory.h"
#include "decls_fp.h"
#include "decls_mpfr.h"

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

static bool isRuntimeFunction(const Function &F) {
    StringRef N = F.getName();
    return N == "shadow_store_double" ||
           N == "shadow_load_double"  ||
           N == "shadow_store_float"  ||
           N == "shadow_load_float";
}

// void handleStore(StoreInst *SI, utils::RuntimeFns &rt, 
//                 DenseMap<const Value*, Value*> &ErrorMap) {
//     llvm::Value *val = SI->getValueOperand();
//     if (!val->getType()->isDoubleTy() && !val->getType()->isFloatTy()) {
//         return;
//     }
//     llvm::Value *ptr = SI->getPointerOperand();
//     llvm::Value *dx = getError(val, rt.ZeroF, rt.ZeroD, ErrorMap);
//     IRBuilder<> AfterSI(SI->getNextNode());
//     if (val->getType()->isDoubleTy()) {
//         AfterSI.CreateCall(rt.ShadowStoreD, {ptr, val, dx});
//     }
//     else {
//         AfterSI.CreateCall(rt.ShadowStoreF, {ptr, val, dx});
//     }
// }

// void handleLoad(LoadInst *LI, utils::RuntimeFns &rt, 
//                 DenseMap<const Value*, Value*> &ErrorMap) {
//     if (!LI->getType()->isDoubleTy() && !LI->getType()->isFloatTy()) {
//         return;
//     }
//     llvm::Value *ptr = LI->getPointerOperand();
//     llvm::Value *dx;
//     IRBuilder<> AfterLI(LI->getNextNode());
//     if (LI->getType()->isDoubleTy()) {
//         dx = AfterLI.CreateCall(rt.ShadowLoadD, {ptr});
//     }
//     else {
//         dx = AfterLI.CreateCall(rt.ShadowLoadF, {ptr});
//     }
//     ErrorMap[LI] = dx;
// }

// bool handleIntrinsic(IntrinsicInst *II, utils::RuntimeFns &rt, utils::RuntimeMPFRFns &rt_mpfr,
//                 DenseMap<const Value*, Value*> &ErrorMap) {

//     if (!II->getType()->isDoubleTy() && !II->getType()->isFloatTy()) {
//         return false;
//     }
//     IRBuilder<> AfterII(II->getNextNode());
//     //TODO: Exception Handling(arg0 < 0)
//     if (II->getIntrinsicID() == Intrinsic::sqrt) {
//         Value *arg0 = II->getArgOperand(0);
//         Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);

//         Value *x = II;
//         Value *negVal = AfterII.CreateFNeg(x, "sqrt.negval");
//         Value *fma = AfterII.CreateIntrinsic(
//             Intrinsic::fma,
//             {x->getType()},
//             {negVal, x, arg0},
//             nullptr,
//             "sqrt.fma"
//         );
//         Value *num = AfterII.CreateFAdd(arg0_err, fma, "sqrt.num");
//         Value *two = ConstantFP::get(x->getType(), 2.0);
//         Value *den = AfterII.CreateFMul(two, x, "sqrt.den");
//         Value *dx = AfterII.CreateFDiv(num, den, "sqrt.err");
//         ErrorMap[II] = dx;
//         // Value *fmt = AfterII.CreateGlobalStringPtr("sqrt_II: x=%f, dx=%e\n");
//         // AfterII.CreateCall(rt.Printf, {fmt, x, dx});
//         return true;
//     }
//     else if (II->getIntrinsicID() == Intrinsic::fabs) {
//         Value *arg0 = II->getArgOperand(0);
//         Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
//         if (II->getType()->isDoubleTy()) {
//             Value *ret = AfterII.CreateCall(rt_mpfr.PropFabsDError, {arg0, arg0_err});
//             // Value *x = Builder.CreateExtractValue(ret, {0}, "fabs.val");
//             Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabs.err");

//             ErrorMap[II] = dx;
//         }
//         else if (II->getType()->isFloatTy()) {
//             Value *ret = AfterII.CreateCall(rt_mpfr.PropFabsFError, {arg0, arg0_err});
//             // Value *x = Builder.CreateExtractValue(ret, {0}, "fabsf.val");
//             Value *dx = AfterII.CreateExtractValue(ret, {1}, "fabsf.err");

//             ErrorMap[II] = dx;
//         }
//         return true;
//     }
//     else {
//         return false;
//     }
// }

// bool handleExternal(CallInst *CI, utils::RuntimeFns &rt, utils::RuntimeMPFRFns &rt_mpfr,
//                 DenseMap<const Value*, Value*> &ErrorMap) {

//     if (!CI->getType()->isDoubleTy() && !CI->getType()->isFloatTy()) {
//         return false;
//     }
//     IRBuilder<> AfterCI(CI->getNextNode());
//     if (Function *Callee = CI->getCalledFunction()) {
//         StringRef N = Callee->getName();
//         //TODO: Exception Handling(arg0 < 0)
//         if (N.contains("sqrt")) {
//             Value *arg0 = CI->getArgOperand(0);
//             Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
//             Value *x = CI;
//             Value *negVal = AfterCI.CreateFNeg(x, "sqrt.negval");
//             Value *fma = AfterCI.CreateIntrinsic(
//                 Intrinsic::fma,
//                 {x->getType()},
//                 {negVal, x, arg0},
//                 nullptr,
//                 "sqrt.fma"
//             );
//             Value *num = AfterCI.CreateFAdd(arg0_err, fma, "sqrt.num");
//             Value *two = ConstantFP::get(x->getType(), 2.0);
//             Value *den = AfterCI.CreateFMul(two, x, "sqrt.den");
//             Value *dx = AfterCI.CreateFDiv(num, den, "sqrt.err");
//             ErrorMap[CI] = dx;
//             // Value *fmt = AfterCI.CreateGlobalStringPtr("sqrt_CI: x=%f, dx=%e\n");
//             // AfterCI.CreateCall(rt.Printf, {fmt, x, dx});
//         }
//         if (N == "expf") {
//             Value *arg0 = CI->getArgOperand(0);
//             Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
//             Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpFError, {arg0, arg0_err});
//             // Value *x = Builder.CreateExtractValue(ret, {0}, "expf.val");
//             Value *dx = AfterCI.CreateExtractValue(ret, {1}, "expf.err");
//             ErrorMap[CI] = dx;
//         }
//         else if (N == "exp") {
//             Value *arg0 = CI->getArgOperand(0);
//             Value *arg0_err = getError(arg0, rt.ZeroF, rt.ZeroD, ErrorMap);
//             Value *ret = AfterCI.CreateCall(rt_mpfr.PropExpDError, {arg0, arg0_err});
//             // Value *x = Builder.CreateExtractValue(ret, {0}, "exp.val");
//             Value *dx = AfterCI.CreateExtractValue(ret, {1}, "exp.err");
//             ErrorMap[CI] = dx;
//         }
//         else {
//             return false;
//         }
//         return true;
//     }
//     return false;
// }

// void handleBinary(Instruction *BO, Constant *ZeroF, Constant *ZeroD,
//                 DenseMap<const Value*, Value*> &ErrorMap) {
    
//     if (!BO->getType()->isDoubleTy() && !BO->getType()->isFloatTy()) {
//         return;
//     }
//     Value *opr0 = BO->getOperand(0);
//     Value *opr1 = BO->getOperand(1);
//     Value *opr0_err = getError(opr0, ZeroF, ZeroD, ErrorMap);
//     Value *opr1_err = getError(opr1, ZeroF, ZeroD, ErrorMap);
//     IRBuilder<> AfterBO(BO->getNextNode());
//     switch (BO->getOpcode()) {
//         case Instruction::FAdd: {
//             Value *x = BO;
//             Value *bp = AfterBO.CreateFSub(x, opr0, "fadd.bp");
//             Value *ap = AfterBO.CreateFSub(x, bp, "fadd.ap");
//             Value *da = AfterBO.CreateFSub(opr0, ap, "fadd.da");
//             Value *db = AfterBO.CreateFSub(opr1, bp, "fadd.db");
//             Value *dab = AfterBO.CreateFAdd(db, da, "fadd.dab");
//             Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fadd.tmp");
//             Value *dx = AfterBO.CreateFAdd(opr1_err, tmp, "fadd.err");
//             ErrorMap[BO] = dx;
//             // Value *fmt = Builder.CreateGlobalStringPtr("FAdd: x=%f, dx=%e\n");
//             // Builder.CreateCall(rt.Printf, {fmt, x, dx});
//             break;
//         }
//         case Instruction::FSub: {
//             Value *x = BO;
//             Value *bp = AfterBO.CreateFSub(x, opr0, "fsub.bp");
//             Value *ap = AfterBO.CreateFSub(x, bp, "fsub.ap");
//             Value *da = AfterBO.CreateFSub(opr0, ap, "fsub.da");
//             Value *db = AfterBO.CreateFAdd(opr1, bp, "fsub.db");
//             Value *dab = AfterBO.CreateFSub(da, db, "fsub.dab");
//             Value *tmp = AfterBO.CreateFAdd(opr0_err, dab, "fsub.tmp");
//             Value *dx = AfterBO.CreateFSub(tmp, opr1_err, "fsub.err");
//             ErrorMap[BO] = dx;
//             // Value *fmt = Builder.CreateGlobalStringPtr("FSub: x=%f, dx=%e\n");
//             // Builder.CreateCall(rt.Printf, {fmt, x, dx});
//             break;
//         }
//         case Instruction::FMul: {
//             Value *x = BO;
//             Value *negVal = AfterBO.CreateFNeg(x, "mul.negval");
//             Value *fma = AfterBO.CreateIntrinsic(
//                 Intrinsic::fma,
//                 {opr0->getType()},
//                 {opr0, opr1, negVal},
//                 nullptr,
//                 "mul.fma"
//             );
//             Value *adb = AfterBO.CreateFMul(opr0, opr1_err, "mul.adb");
//             Value *tmp = AfterBO.CreateFAdd(fma, adb, "mul.tmp");
//             Value *bda = AfterBO.CreateFMul(opr1, opr0_err, "mul.bda");
//             Value *dx = AfterBO.CreateFAdd(tmp, bda, "mul.err");
//             ErrorMap[BO] = dx;
//             // Value *fmt = Builder.CreateGlobalStringPtr("FMul: x=%f, dx=%e\n");
//             // Builder.CreateCall(rt.Printf, {fmt, x, dx});
//             break;
//         }
//         case Instruction::FDiv: {
//             Value *x = BO;
//             Value *invopr0 = AfterBO.CreateFNeg(opr0, "div.invopr0");
//             Value *fma = AfterBO.CreateIntrinsic(
//                 Intrinsic::fma,
//                 {invopr0->getType()},
//                 {x, opr1, invopr0},
//                 nullptr,
//                 "div.fma"
//             );
//             Value *da = AfterBO.CreateFSub(opr0_err, fma);
//             Value *invval = AfterBO.CreateFNeg(x, "div.invval");
//             Value *numer = AfterBO.CreateIntrinsic(
//                 Intrinsic::fma,
//                 {invval->getType()},
//                 {invval, opr1_err, da},
//                 nullptr,
//                 "div.numer"
//             );
//             Value *denom = AfterBO.CreateFAdd(opr1, opr1_err, "div.denom");
//             Value *dx = AfterBO.CreateFDiv(numer, denom, "div.err");
//             ErrorMap[BO] = dx;
//             // Value *fmt = Builder.CreateGlobalStringPtr("FDib: x=%f, dx=%e\n");
//             // Builder.CreateCall(rt.Printf, {fmt, x, dx});
//             break;
//         }
//         default:
//             break;
//     }
// }

void runOnModule(llvm::Module &M) {
    utils::RuntimeFns rt(M);
    utils::RuntimeMPFRFns rt_mpfr(M);
    auto &Ctx = M.getContext();

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        if (isRuntimeFunction(F)) continue;

        DenseMap<const Value*, Value*> ErrorMap;

        SmallVector<Instruction*, 128> WorkList;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                WorkList.push_back(&I);
            }
        }
        for (auto *I : WorkList) {
            if (isa<PHINode>(I)) continue;

            if (auto *LI = dyn_cast<LoadInst>(I)) {
                if(handleLoad(LI, rt, ErrorMap)) {
                    continue;
                }
                // handleLoad(LI, rt, ErrorMap);
                // continue;
            }
            if (auto *SI = dyn_cast<StoreInst>(I)) {
                if(handleStore(SI, rt, ErrorMap)) {
                    continue;
                }
            }
            if (auto *II = dyn_cast<IntrinsicInst>(I)) {
                if(handleIntrinsic(II, rt.ZeroF, rt.ZeroD, rt_mpfr, ErrorMap)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<CallInst>(I)) {
                if(handleExternal(CI, rt.ZeroF, rt.ZeroD, rt_mpfr, ErrorMap)) {
                    continue;
                }
            }
            if (auto *BO = dyn_cast<BinaryOperator>(I)) {
                if(handleBinary(BO, rt.ZeroF, rt.ZeroD, ErrorMap)) {
                    continue;
                }
                // handleBinary(BO, rt.ZeroF, rt.ZeroD, ErrorMap);
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