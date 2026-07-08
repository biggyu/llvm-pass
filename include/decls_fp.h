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
        llvm::Type *I64Ty;
        llvm::Type *FloatTy;
        llvm::Type *DoubleTy;
        llvm::Type *PtrTy;
        llvm::Type *FnPtrTy;

        llvm::StructType *ShadowEntryTy;

        llvm::Constant *ZeroF;
        llvm::Constant *ZeroD;

        llvm::FunctionCallee Printf;
        llvm::FunctionCallee ShadowStoreD;
        llvm::FunctionCallee ShadowStoreF;
        llvm::FunctionCallee ShadowLoadD;
        llvm::FunctionCallee ShadowLoadF;
        llvm::FunctionCallee ShadowStackPush;
        llvm::FunctionCallee ShadowStackPop;

        llvm::FunctionCallee CheckConvSI;
        llvm::FunctionCallee CheckConvUI;
        llvm::FunctionCallee CheckBranch;
        llvm::FunctionCallee CheckError;
        llvm::FunctionCallee Atexit;
        llvm::FunctionCallee ReportDebugSummary;
        llvm::FunctionCallee ConditionNumber;
        // llvm::FunctionCallee ConditionNumberD;
        // llvm::FunctionCallee ConditionNumberF;

        llvm::FunctionType *PrintfTy;
        llvm::FunctionType *ShadowStoreFTy;
        llvm::FunctionType *ShadowStoreDTy;
        llvm::FunctionType *ShadowLoadFTy;
        llvm::FunctionType *ShadowLoadDTy;
        llvm::FunctionType *ShadowStackPushTy;
        llvm::FunctionType *ShadowStackPopTy;

        llvm::FunctionType *CheckConvSITy;
        llvm::FunctionType *CheckConvUITy;
        llvm::FunctionType *CheckBranchTy;
        llvm::FunctionType *CheckErrorTy;
        llvm::FunctionType *AtexitTy;
        llvm::FunctionType *ReportDebugSummaryTy;
        // llvm::FunctionType *ConditionNumberFTy;
        // llvm::FunctionType *ConditionNumberDTy;
        llvm::FunctionType *ConditionNumberTy;
        
        explicit RuntimeFns(llvm::Module &Mod);
    };
}