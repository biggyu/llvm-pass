#include "DebugCheck.h"
#include <cstdint>
#include <string>
#include "llvm/IR/DebugInfoMetadata.h"
#include "../runtime/fp_condition.h"

cl::opt<bool> EnableDebugChecks(
    "fp-debug-checks",
    cl::desc("Enable floating-point debug checks after error propagation"),
    cl::init(false)
);

cl::opt<bool> EnableDebugAutoReport(
    "fp-debug-auto-report",
    cl::desc("Insert report_debug_summary() in the input code"),
    cl::init(false)
);

cl::opt<int> DebugMetrics(
    "fp-debug-metric",
    cl::desc("Metric for floating-point debug checks"),
    cl::init(0)
);

DenseSet<uint32_t> RegisteredSites;

static bool isRuntimeFunction(const Function &F) {
    StringRef N = F.getName();
    return N == "shadow_store"              ||
           N == "shadow_load"               ||
           N == "check_error_float"         ||
           N == "check_error_double"        ||
           N == "register_fp_site"          ||
           N == "report_debug_summary"      ||
           N == "condition_number_float"    ||
           N == "condition_number_double";
}

static uint32_t hash32string(llvm::StringRef S) {
    uint32_t H = 2166136261u;
    for (char c : S) {
        H ^= static_cast<unsigned char>(c);
        H *= 16777619u;
    }
    return H;
}

uint32_t getSiteId(const llvm::Instruction *I) {
    std::string Key;
    if (llvm::DILocation *Loc = I->getDebugLoc()) {
        Key += Loc->getDirectory().str();
        Key += "/";
        Key += Loc->getFilename().str();
        Key += ":";
        Key += std::to_string(Loc->getLine());
        Key += ":";
        Key += std::to_string(Loc->getColumn());
    }
    else {
        Key += "<no-debug-loc>";
    }
    Key += ":";
    Key += std::to_string(I->getOpcode());
    Key += ":";
    
    llvm::raw_string_ostream OS(Key);
    I->getType()->print(OS);
    OS.flush();
    return hash32string(Key);
}

bool insertCheckError(IRBuilder<> &B,
                    const DSLValues &aDsl, 
                    const DSLValues &bDsl, 
                    const DSLValues &xDsl, 
                    Instruction *Site, FpOp op,
                    utils::RuntimeFns &rt) {
    uint32_t id = getSiteId(Site);
    Value *SiteId = ConstantInt::get(rt.I32Ty, id);
    Value *Metric = ConstantInt::get(rt.I32Ty, DebugMetrics);

    DebugLoc DL = Site->getDebugLoc();

    std::string File = "<unknown>";
    int Line = 0, Col = 0;

    if (DL) {
        File = DL.get()->getFilename().str();
        Line = DL.get()->getLine();
        Col = DL.get()->getColumn();
        // Line = DL->getLine();
        // Col = DL->getColumn();
    }

    std::string Func = Site->getFunction()->getName().str();
    std::string Opcode = Site->getOpcodeName();

    Value *FileStr = B.CreateGlobalStringPtr(File);
    Value *FuncStr = B.CreateGlobalStringPtr(Func);
    Value *OpcodeStr = B.CreateGlobalStringPtr(Opcode);

    if (RegisteredSites.insert(id).second) {
        B.CreateCall(rt.RegisterFPSite, {
            SiteId,
            FuncStr,
            FileStr,
            ConstantInt::get(rt.I32Ty, Line),
            ConstantInt::get(rt.I32Ty, Col),
            OpcodeStr,
        });
    }

    bool emitCond = (op != FpOp::Mul && op != FpOp::Div && op != FpOp::Sqrt && op != FpOp::Cbrt && op != FpOp::Unknown);

    if (xDsl.xhat->getType()->isDoubleTy()) {
        if (emitCond) {
            B.CreateCall(rt.ConditionNumberD, {ConstantInt::get(rt.I32Ty, (uint32_t)op), aDsl.xhat, aDsl.error, bDsl.xhat, bDsl.error, aDsl.isExact, bDsl.isExact, SiteId});
        }
        B.CreateCall(rt.CheckErrorD, {xDsl.xhat, xDsl.error, SiteId, Metric});
        return true;
    }
    if (xDsl.xhat->getType()->isFloatTy()) {
        if (emitCond) {
            B.CreateCall(rt.ConditionNumberF, {ConstantInt::get(rt.I32Ty, (uint32_t)op), aDsl.xhat, aDsl.error, bDsl.xhat, bDsl.error, aDsl.isExact, bDsl.isExact, SiteId});
        }
        B.CreateCall(rt.CheckErrorF, {xDsl.xhat, xDsl.error, SiteId, Metric});
        return true;
    }
    return false;
}

bool insertReportInMain(Module &M, utils::RuntimeFns &rt) {
    Function *Main = M.getFunction("main");
    if(!Main) {
        return false;
    }

    for (BasicBlock &BB : *Main) {
        Instruction *Term = BB.getTerminator();
        if (auto *RI = dyn_cast<ReturnInst>(Term)) {
            IRBuilder<> B(RI);
            B.CreateCall(rt.ReportDebugSummary);
        }
    }
    return true;
}

void insertReportDebugSummary(Module &M, utils::RuntimeFns &rt) {
    if (insertReportInMain(M, rt)) {
        return;
    }

    for (Function &F : M) {
        if (F.isDeclaration() || isRuntimeFunction(F) || !F.getName().starts_with("ex")) {
            continue;
        }
        for (BasicBlock &BB : F) {
            if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
                IRBuilder<> B(RI);
                B.CreateCall(rt.ReportDebugSummary);
            }
        }
    }
}