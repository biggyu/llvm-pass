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
        llvm::StructType *FpEntryTy;

        llvm::FunctionCallee PropSinError;
        llvm::FunctionCallee PropCosError;
        llvm::FunctionCallee PropTanError;
        llvm::FunctionCallee PropAsinError;
        llvm::FunctionCallee PropAcosError;
        llvm::FunctionCallee PropAtanError;
        llvm::FunctionCallee PropLogError;
        llvm::FunctionCallee PropExpError;
        llvm::FunctionCallee PropPowError;

        llvm::FunctionType *PropSinErrorTy;
        llvm::FunctionType *PropCosErrorTy;
        llvm::FunctionType *PropTanErrorTy;
        llvm::FunctionType *PropAsinErrorTy;
        llvm::FunctionType *PropAcosErrorTy;
        llvm::FunctionType *PropAtanErrorTy;
        llvm::FunctionType *PropLogErrorTy;
        llvm::FunctionType *PropExpErrorTy;
        llvm::FunctionType *PropPowErrorTy;

        explicit RuntimeMPFRFns(llvm::Module &Mod);
    };
}