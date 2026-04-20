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

        explicit RuntimeMPFRFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
            VoidTy = llvm::Type::getVoidTy(Ctx);
            // I32Ty = llvm::Type::getInt32Ty(Ctx);
            FloatTy = llvm::Type::getFloatTy(Ctx);
            DoubleTy = llvm::Type::getDoubleTy(Ctx);
            PtrTy = llvm::PointerType::getUnqual(Ctx);
            FpEntryFTy = llvm::StructType::create(Ctx, "fp_entry_f");
            FpEntryFTy->setBody({FloatTy, FloatTy}, false);
            FpEntryDTy = llvm::StructType::create(Ctx, "fp_entry_d");
            FpEntryDTy->setBody({DoubleTy, DoubleTy}, false);

            PropExpFErrorTy = llvm::FunctionType::get(
                FpEntryFTy,
                {FloatTy, FloatTy},
                false
            );
            PropExpDErrorTy = llvm::FunctionType::get(
                FpEntryDTy,
                {DoubleTy, DoubleTy},
                false
            );
            PropFabsFErrorTy = llvm::FunctionType::get(
                FpEntryFTy,
                {FloatTy, FloatTy},
                false
            );
            PropFabsDErrorTy = llvm::FunctionType::get(
                FpEntryDTy,
                {DoubleTy, DoubleTy},
                false
            );

            PropExpFError = M.getOrInsertFunction("PropExpFError", PropExpFErrorTy);
            PropExpDError = M.getOrInsertFunction("PropExpDError", PropExpDErrorTy);
            PropFabsFError = M.getOrInsertFunction("PropFabsFError", PropFabsFErrorTy);
            PropFabsDError = M.getOrInsertFunction("PropFabsDError", PropFabsDErrorTy);
        }

    };
}