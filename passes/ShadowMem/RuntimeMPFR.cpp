#include "decls_mpfr.h"

using namespace llvm;

namespace utils {
    RuntimeMPFRFns::RuntimeMPFRFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        // I32Ty = llvm::Type::getInt32Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);
        FpEntryFTy = llvm::StructType::create(Ctx, "fp_entryF");
        FpEntryFTy->setBody({FloatTy, FloatTy}, false);
        FpEntryDTy = llvm::StructType::create(Ctx, "fp_entryD");
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
}