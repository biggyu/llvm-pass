#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"

namespace utils {
    struct RuntimeMPFRFns {
        llvm::Module &M;
        llvm::LLVMContext &Ctx;

        llvm::Type *VoidTy;
        llvm::Type *FloatTy;
        llvm::Type *DoubleTy;
        llvm::Type *PtrTy;
        llvm::StructType *FpEntryFTy;
        llvm::StructType *FpEntryDTy;

        // llvm::FunctionCallee PropSinFError;
        // llvm::FunctionCallee PropSinDError;
        // llvm::FunctionCallee PropCosFError;
        // llvm::FunctionCallee PropCosDError;
        // llvm::FunctionCallee PropTanFError;
        // llvm::FunctionCallee PropTanDError;
        // llvm::FunctionCallee PropAsinFError;
        // llvm::FunctionCallee PropAsinDError;
        // llvm::FunctionCallee PropAcosFError;
        // llvm::FunctionCallee PropAcosDError;
        // llvm::FunctionCallee PropAtanFError;
        // llvm::FunctionCallee PropAtanDError;
        // llvm::FunctionCallee PropLogFError;
        // llvm::FunctionCallee PropLogDError;
        llvm::FunctionCallee PropExpFError;
        llvm::FunctionCallee PropExpDError;
        // llvm::FunctionCallee PropPowFError;
        // llvm::FunctionCallee PropPowDError;
        llvm::FunctionCallee PropFabsFError;
        llvm::FunctionCallee PropFabsDError;

        // llvm::FunctionType *PropSinFErrorTy;
        // llvm::FunctionType *PropSinDErrorTy;
        // llvm::FunctionType *PropCosFErrorTy;
        // llvm::FunctionType *PropCosDErrorTy;
        // llvm::FunctionType *PropTanFErrorTy;
        // llvm::FunctionType *PropTanDErrorTy;
        // llvm::FunctionType *PropAsinFErrorTy;
        // llvm::FunctionType *PropAsinDErrorTy;
        // llvm::FunctionType *PropAcosFErrorTy;
        // llvm::FunctionType *PropAcosDErrorTy;
        // llvm::FunctionType *PropAtanFErrorTy;
        // llvm::FunctionType *PropAtanDErrorTy;
        // llvm::FunctionType *PropLogFErrorTy;
        // llvm::FunctionType *PropLogDErrorTy;
        llvm::FunctionType *PropExpFErrorTy;
        llvm::FunctionType *PropExpDErrorTy;
        // llvm::FunctionType *PropPowFErrorTy;
        // llvm::FunctionType *PropPowDErrorTy;
        llvm::FunctionType *PropFabsFErrorTy;
        llvm::FunctionType *PropFabsDErrorTy;

        explicit RuntimeMPFRFns(llvm::Module &Mod);
    };
}