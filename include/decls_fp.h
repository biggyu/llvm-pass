#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"

namespace utils {
    struct RuntimeFns {
        llvm::Module &M;
        llvm::LLVMContext &Ctx;

        llvm::Type *VoidTy;
        llvm::Type *BoolTy;
        llvm::Type *I32Ty;
        llvm::Type *FloatTy;
        llvm::Type *DoubleTy;
        llvm::Type *PtrTy;

        llvm::StructType *ShadowEntryTy;

        llvm::Constant *ZeroF;
        llvm::Constant *ZeroD;

        llvm::FunctionCallee Printf;
        llvm::FunctionCallee ShadowStore;
        llvm::FunctionCallee ShadowLoad;

        llvm::FunctionCallee CheckErrorD;
        llvm::FunctionCallee CheckErrorF;
        llvm::FunctionCallee RegisterFPSite;
        llvm::FunctionCallee ReportDebugSummary;
        llvm::FunctionCallee ConditionNumberD;
        llvm::FunctionCallee ConditionNumberF;

        llvm::FunctionType *PrintfTy;
        llvm::FunctionType *ShadowStoreTy;
        llvm::FunctionType *ShadowLoadTy;

        llvm::FunctionType *CheckErrorFTy;
        llvm::FunctionType *CheckErrorDTy;
        llvm::FunctionType *RegisterFPSiteTy;
        llvm::FunctionType *ReportDebugSummaryTy;
        llvm::FunctionType *ConditionNumberFTy;
        llvm::FunctionType *ConditionNumberDTy;
        
        explicit RuntimeFns(llvm::Module &Mod);
    };
}