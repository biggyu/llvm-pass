#include "decls_mpfr.h"

using namespace llvm;

namespace utils {
    RuntimeMPFRFns::RuntimeMPFRFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        // I32Ty = llvm::Type::getInt32Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);
        FpEntryTy = llvm::StructType::create(Ctx, "fp_entry");
        FpEntryTy->setBody({DoubleTy, DoubleTy}, false);

        PropSinErrorTy = llvm::FunctionType::get(
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropCosErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropTanErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},\
            false
        );
        PropAsinErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropAcosErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropAtanErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropLogErrorTy = llvm::FunctionType::get( 
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropExpErrorTy = llvm::FunctionType::get(
            FpEntryTy,
            {DoubleTy, DoubleTy},
            false
        );
        PropPowErrorTy = llvm::FunctionType::get(
            FpEntryTy,
            {DoubleTy, DoubleTy, DoubleTy, DoubleTy},
            false
        );

        PropSinError = M.getOrInsertFunction("PropSinError", PropSinErrorTy);
        PropCosError = M.getOrInsertFunction("PropCosError", PropCosErrorTy);
        PropTanError = M.getOrInsertFunction("PropTanError", PropTanErrorTy);
        PropAsinError = M.getOrInsertFunction("PropAsinError", PropAsinErrorTy);
        PropAcosError = M.getOrInsertFunction("PropAcosError", PropAcosErrorTy);
        PropAtanError = M.getOrInsertFunction("PropAtanError", PropAtanErrorTy);
        PropLogError = M.getOrInsertFunction("PropLogError", PropLogErrorTy);
        PropExpError = M.getOrInsertFunction("PropExpError", PropExpErrorTy);
        PropPowError = M.getOrInsertFunction("PropPowError", PropPowErrorTy);
    }
}