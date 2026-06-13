#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "ShadowMemPass.h"
#include "ErrorProp.h"
#include "ShadowMemory.h"
#include "DebugCheck.h"
#include "decls_fp.h"
#include "decls_mpfr.h"

using namespace llvm;

static bool isRuntimeFunction(const Function &F) {
    StringRef N = F.getName();
    return N == "shadow_store_double" ||
           N == "shadow_load_double"  ||
           N == "shadow_store_float"  ||
           N == "shadow_load_float"   ||
           N == "check_error_float"   ||
           N == "check_error_double"  ||
           N == "register_fp_site"    ||
           N == "report_debug_summary";
}

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
                if(handleIntrinsic(II, rt, rt_mpfr, ErrorMap)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<CallInst>(I)) {
                if(handleExternal(CI, rt, rt_mpfr, ErrorMap)) {
                    continue;
                }
            }
            if (auto *BO = dyn_cast<BinaryOperator>(I)) {
                if(handleBinary(BO, rt, ErrorMap)) {
                    continue;
                }
                // handleBinary(BO, rt.ZeroF, rt.ZeroD, ErrorMap);
            }
        }
    }
    if (EnableDebugChecks) {
    // if (EnableDebugChecks && EnableDebugAutoReport) {
        insertReportDebugSummary(M, rt);
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