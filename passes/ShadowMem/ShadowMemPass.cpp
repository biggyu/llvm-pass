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

        Value *sharedLoadOut = nullptr;
        Value *sharedExtOut = nullptr;
        {
            IRBuilder<> Entry(&*F.getEntryBlock().getFirstInsertionPt());
            sharedLoadOut = Entry.CreateAlloca(rt.ShadowEntryTy, nullptr, "shared.load.out");
            sharedExtOut = Entry.CreateAlloca(rt.ShadowEntryTy, nullptr, "shared.external.out");
        }

        if (F.getName() != "main") {
            IRBuilder<> Entry(&*F.getEntryBlock().getFirstInsertionPt());
            for (Argument &param : F.args()) {
                if (param.getType()->isDoubleTy() || param.getType()->isFloatTy()) {
                    Value *outPtr = Entry.CreateAlloca(rt.ShadowEntryTy, nullptr, "pop.out");
                    Entry.CreateCall(rt.ShadowStackPop, {outPtr});
                    Value *entry = Entry.CreateLoad(rt.ShadowEntryTy, outPtr);
                    DSLMap[&param] = extractDSL(Entry, entry);
                }
            }
        }
        struct PHIShadow {
            PHINode *xhat, *rhat, *fpval, *relerr;
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
                ps.fpval = B.CreatePHI(rt.DoubleTy, n, "phi.fpval");
                ps.relerr = B.CreatePHI(rt.DoubleTy, n, "phi.relerr");
                PHIMap[&PN] = ps;

                DSLValues d;
                d.xhat = ps.xhat;
                d.rhat = ps.rhat;
                d.fpval = ps.fpval;
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
                if (handleLoad(LI, rt, DSLMap, sharedLoadOut)) {
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
                if (handleExternal(CI, rt, rt_mpfr, sharedExtOut, DSLMap, SiteDescs)) {
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
                Value *fpv = inDSL.fpval;
                if (inDSL.fpval->getType()->isFloatTy()) {
                    fpv = PredB.CreateFPExt(fpv, rt.DoubleTy);
                }
                ps.fpval->addIncoming(fpv, inBB);
                // ps.fpval->addIncoming(inDSL.fpval, inBB);
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