#include "DebugCheck.h"
#include <cstdint>
#include "llvm/IR/DebugInfoMetadata.h"

cl::opt<bool> EnableDebugChecks(
    "fp-debug-checks",
    cl::desc("Enable floating-point debug checks after error propagation"),
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
        Key += Loc->getFilename().str();
        Key += ":";
        Key += std::to_string(Loc->getLine());
        Key += ":";
        Key += std::to_string(Loc->getColumn());
        Key += ":";
        Key += I->getOpcode();
    }
    else {
        Key += I->getFunction()->getName().str();
        Key += ":";
        Key += I->getOpcode();
    }
    return hash32string(Key);
    
}

bool insertCheckError(IRBuilder<> &B,
                    Value *x, Value *dx,
                    Instruction *Site, utils::RuntimeFns &rt) {
    uint32_t id = getSiteId(Site);
    Value *SiteId = ConstantInt::get(rt.I32Ty, id);
    Value *Metric = ConstantInt::get(rt.I32Ty, DebugMetrics);

    if (x->getType()->isDoubleTy()) {
        B.CreateCall(rt.CheckErrorD, {x, dx, SiteId, Metric});
        return true;
    }
    if (x->getType()->isFloatTy()) {
        B.CreateCall(rt.CheckErrorF, {x, dx, SiteId, Metric});
        return true;
    }
    return false;
}