#include "DebugCheck.h"
#include <cstdint>
#include <string>
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

// static std::unordered_map<uint32_t, utils::SiteDesc> g_siteDesc;

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

void recordSiteDesc(uint32_t id, Instruction *I,
                    std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    if (SiteDescs.count(id)) {
        return;
    }
    utils::SiteDesc d;
    d.id = id;
    if (DILocation *L = I->getDebugLoc()) {
        d.file = L->getFilename().str();
        d.line = L->getLine();
        d.col = L->getColumn();
    }
    else {
        d.file = "<unknown>";
        d.line = 0;
        d.col = 0;
    }
    d.func = I->getFunction()->getName().str();
    d.opcode = I->getOpcodeName();
    SiteDescs[id] = std::move(d);
}

void insertCheckError(IRBuilder<> &B,
                    const DSLValues &aDsl, const DSLValues &bDsl, 
                    DSLValues &xDsl, 
                    Instruction *Site, FpOp op,
                    utils::RuntimeFns &rt,
                    std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    uint32_t id = getSiteId(Site);
    Value *SiteId = ConstantInt::get(rt.I32Ty, id);
    Value *Metric = ConstantInt::get(rt.I32Ty, DebugMetrics);

    recordSiteDesc(id, Site, SiteDescs);

    bool emitCond = (
        op != FpOp::Mul && op != FpOp::Div && 
        op != FpOp::Sqrt && op != FpOp::Cbrt && 
        op != FpOp::Branch && op != FpOp::ConvSI && 
        op != FpOp::ConvUI && op != FpOp::Unknown);

    if (emitCond) {
        Value *Ex = B.CreateCall(rt.ConditionNumber, {
            ConstantInt::get(rt.I32Ty, (uint32_t)op), 
            aDsl.xhat, aDsl.relerr, 
            bDsl.xhat, bDsl.relerr, 
            aDsl.fpval, bDsl.fpval, 
            SiteId
        });
        Value *ci = ConstantFP::get(rt.DoubleTy, std::numeric_limits<double>::epsilon() / 2.0);
        xDsl.relerr = B.CreateFAdd(Ex, ci, "x.relerr");
    }
    B.CreateCall(rt.CheckError, {xDsl.xhat, xDsl.rhat, SiteId, Metric});
    // return false;
}

void insertCheckBranch(IRBuilder<> &B,
                    const DSLValues &aDsl, const DSLValues &bDsl, 
                    Value* pred, Instruction *Site, FpOp op,
                    utils::RuntimeFns &rt,
                    std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    uint32_t id = getSiteId(Site);
    Value *SiteId = ConstantInt::get(rt.I32Ty, id);
    Value *Metric = ConstantInt::get(rt.I32Ty, DebugMetrics);

    recordSiteDesc(id, Site, SiteDescs);
    
    B.CreateCall(rt.CheckBranch, {aDsl.xhat, aDsl.rhat, bDsl.xhat, bDsl.rhat, pred, Site, SiteId});
}

void insertCheckConv(IRBuilder<> &B, DSLValues &xDsl, 
                    Value *val, Instruction *Site, FpOp op,
                    utils::RuntimeFns &rt,
                    std::unordered_map<uint32_t, utils::SiteDesc> &SiteDescs) {
    uint32_t id = getSiteId(Site);
    Value *SiteId = ConstantInt::get(rt.I32Ty, id);
    Value *Metric = ConstantInt::get(rt.I32Ty, DebugMetrics);

    recordSiteDesc(id, Site, SiteDescs);

    if (op == FpOp::ConvSI) {
        B.CreateCall(rt.CheckConvSI, {val, xDsl.xhat, xDsl.rhat, SiteId});
    }
    else {
        B.CreateCall(rt.CheckConvUI, {val, xDsl.xhat, xDsl.rhat, SiteId});
    }
}

void emitRegisterAllSites(Module &M, std::unordered_map<uint32_t, utils::SiteDesc> SiteDescs, utils::RuntimeFns &rt) {
    LLVMContext &Ctx = M.getContext();
    FunctionType *CtorTy = FunctionType::get(rt.VoidTy, false);
    Function *Ctor = Function::Create(CtorTy, GlobalValue::InternalLinkage, "__fp_register_call", &M);
    IRBuilder B(BasicBlock::Create(Ctx, "entry", Ctor));
    
    for (const auto &kv : SiteDescs) {
        Value *FuncStr = B.CreateGlobalStringPtr(kv.second.func);
        Value *FileStr = B.CreateGlobalStringPtr(kv.second.file);
        Value *OpStr = B.CreateGlobalStringPtr(kv.second.opcode);
        B.CreateCall(rt.RegisterFPSite, {
            ConstantInt::get(rt.I32Ty, kv.second.id), 
            FuncStr,
            FileStr,
            ConstantInt::get(rt.I32Ty, kv.second.line),
            ConstantInt::get(rt.I32Ty, kv.second.col),
            OpStr,
        });
    }
    B.CreateRetVoid();
    appendToGlobalCtors(M, Ctor, 0);
}

bool insertReportInMain(Module &M, utils::RuntimeFns &rt) {
    Function *Main = M.getFunction("main");
    if(!Main) {
        return false;
    }
    IRBuilder<> B(&*Main->getEntryBlock().getFirstInsertionPt());
    B.CreateCall(rt.Atexit, {rt.ReportDebugSummary.getCallee()});
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