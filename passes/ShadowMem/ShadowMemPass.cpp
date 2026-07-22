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
#include "DSLValues.h"

using namespace llvm;

// bool isRuntimeFunction(const Function &F) {
//     StringRef N = F.getName();
//     return N == "shadow_store_double" ||
//            N == "shadow_store_float"  ||
//            N == "shadow_load_double"  ||
//            N == "shadow_load_float"   ||
//            N == "shadow_stack_push"   ||
//            N == "shadow_stack_pop"    ||
//            N == "check_conv_ui"       ||
//            N == "check_conv_si"       ||
//            N == "check_branch"        ||
//            N == "check_error"         ||
//            N == "report_debug_summary";
// }

void runOnModule(llvm::Module &M) {
    utils::RuntimeFns rt(M);
    utils::RuntimeMPFRFns rt_mpfr(M);
    auto &Ctx = M.getContext();

    std::unordered_map<uint32_t, utils::SiteDesc> SiteDescs;

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        if (isRuntimeFunction(F)) continue;

        // DenseMap<const Value*, Value*> ErrorMap;
        DenseMap<const Value*, DSLValues> DSLMap;

        if (F.getName() != "main") {
            IRBuilder<> Entry(&*F.getEntryBlock().getFirstInsertionPt());
            for (Argument &param : F.args()) {
                if (param.getType()->isDoubleTy() || param.getType()->isFloatTy()) {
                    Value *entry = Entry.CreateCall(rt.ShadowStackPop, {});
                    DSLMap[&param] = extractDSL(Entry, entry);
                }
            }
        }

        SmallVector<Instruction*, 128> WorkList;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                WorkList.push_back(&I);
            }
        }
        for (auto *I : WorkList) {
            if (isa<PHINode>(I)) continue;

            if (auto *LI = dyn_cast<LoadInst>(I)) {
                if (handleLoad(LI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *SI = dyn_cast<StoreInst>(I)) {
                if (handleStore(SI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *RI = dyn_cast<ReturnInst>(I)) {
                if (handleReturn(RI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *II = dyn_cast<IntrinsicInst>(I)) {
                if (handleIntrinsic(II, rt, rt_mpfr, DSLMap, SiteDescs)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<CallInst>(I)) {
                if (handleExternal(CI, rt, rt_mpfr, DSLMap, SiteDescs)) {
                    continue;
                }
            }
            if (auto *UO = dyn_cast<UnaryOperator>(I)) {
                if (handleUnary(UO, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *BO = dyn_cast<BinaryOperator>(I)) {
                if (handleBinary(BO, rt, DSLMap, SiteDescs)) {
                    continue;
                }
            }
            if (auto *FC = dyn_cast<FCmpInst>(I)) {
                if (handleFCmp(FC, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<FPToSIInst>(I)) {
                if (handleFPToSI(CI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<FPToUIInst>(I)) {
                if (handleFPToUI(CI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *SI = dyn_cast<SIToFPInst>(I)) {
                if (handleSIToFP(SI, rt, DSLMap)) {
                    continue;
                }
            }
            if (auto *UI = dyn_cast<UIToFPInst>(I)) {
                if (handleUIToFP(UI, rt, DSLMap)) {
                    continue;
                }
            }
        }
    }
    if (EnableDebugChecks && !SiteDescs.empty()) {
        emitRegisterAllSites(M, SiteDescs, rt);
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