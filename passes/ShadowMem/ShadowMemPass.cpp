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
        struct PHIShadow {
            PHINode *xhat, *rhat, *ehat, *sign, *isExact, *relerr;
        };
        DenseMap<const PHINode*, PHIShadow> PHIMap;

        for (BasicBlock &BB : F) {
            for (PHINode &PN : BB.phis()) {
                if (!PN.getType()->isFloatTy() && !PN.getType()->isDoubleTy()) {
                    continue;
                }
                unsigned n = PN.getNumIncomingValues();
                IRBuilder<> B(&PN);
                PHIShadow ps;
                ps.xhat = B.CreatePHI(rt.DoubleTy, n, "phi.xhat");
                ps.rhat = B.CreatePHI(rt.DoubleTy, n, "phi.rhat");
                ps.ehat = B.CreatePHI(rt.DoubleTy, n, "phi.ehat");
                ps.sign = B.CreatePHI(rt.BoolTy, n, "phi.s");
                ps.isExact = B.CreatePHI(rt.BoolTy, n, "phi.i");
                ps.relerr = B.CreatePHI(rt.DoubleTy, n, "phi.relerr");
                PHIMap[&PN] = ps;

                DSLValues d;
                d.xhat = ps.xhat;
                d.rhat = ps.rhat;
                d.ehat = ps.ehat;
                d.sign = ps.sign;
                d.isExact = ps.isExact;
                d.relerr = ps.relerr;
                DSLMap[&PN] = d;
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
                if (handleFCmp(FC, rt, DSLMap, SiteDescs)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<FPToSIInst>(I)) {
                if (handleFPToSI(CI, rt, DSLMap, SiteDescs)) {
                    continue;
                }
            }
            if (auto *CI = dyn_cast<FPToUIInst>(I)) {
                if (handleFPToUI(CI, rt, DSLMap, SiteDescs)) {
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
        for (auto &kv : PHIMap) {
            const PHINode *PN = kv.first;
            PHIShadow &ps = kv.second;
            for (unsigned i = 0; i < PN->getNumIncomingValues(); i++) {
                Value *inVal = PN->getIncomingValue(i);
                BasicBlock *inBB = PN->getIncomingBlock(i);
                IRBuilder<> PredB(inBB->getTerminator());
                DSLValues inDSL = getDSL(PredB, inVal, rt, DSLMap);
                ps.xhat->addIncoming(inDSL.xhat, inBB);
                ps.rhat->addIncoming(inDSL.rhat, inBB);
                ps.ehat->addIncoming(inDSL.ehat, inBB);
                ps.sign->addIncoming(inDSL.sign, inBB);
                ps.isExact->addIncoming(inDSL.isExact, inBB);
                ps.relerr->addIncoming(inDSL.relerr, inBB);
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