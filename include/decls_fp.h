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
        llvm::Type *I32Ty;
        llvm::Type *FloatTy;
        llvm::Type *DoubleTy;
        llvm::Type *PtrTy;

        llvm::Constant *ZeroF;
        llvm::Constant *ZeroD;

        llvm::FunctionCallee Printf;
        llvm::FunctionCallee ShadowStoreD;
        llvm::FunctionCallee ShadowStoreF;
        llvm::FunctionCallee ShadowLoadD;
        llvm::FunctionCallee ShadowLoadF;

        llvm::FunctionCallee CheckErrorD;
        llvm::FunctionCallee CheckErrorF;
        llvm::FunctionCallee RegisterFPSite;
        llvm::FunctionCallee ReportDebugSummary;

        llvm::FunctionType *PrintfTy;
        llvm::FunctionType *ShadowStoreFTy;
        llvm::FunctionType *ShadowStoreDTy;
        llvm::FunctionType *ShadowLoadFTy;
        llvm::FunctionType *ShadowLoadDTy;

        llvm::FunctionType *CheckErrorFTy;
        llvm::FunctionType *CheckErrorDTy;
        llvm::FunctionType *RegisterFPSiteTy;
        llvm::FunctionType *ReportDebugSummaryTy;
        
        explicit RuntimeFns(llvm::Module &Mod);
    };
}