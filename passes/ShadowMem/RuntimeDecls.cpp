#include "decls_fp.h"

using namespace llvm;

namespace utils {
    RuntimeFns::RuntimeFns(Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        I32Ty = llvm::Type::getInt32Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);

        ZeroD = llvm::ConstantFP::get(DoubleTy, 0.0);
        ZeroF = llvm::ConstantFP::get(FloatTy, 0.0);

        PrintfTy = llvm::FunctionType::get(
            I32Ty,
            {PtrTy},
            true
        );

        ShadowStoreDTy = llvm::FunctionType::get(
            VoidTy,
            {PtrTy, DoubleTy, DoubleTy},
            false
        );
        ShadowStoreFTy = llvm::FunctionType::get(
            VoidTy,
            {PtrTy, FloatTy, FloatTy},
            false
        );
        ShadowLoadDTy = llvm::FunctionType::get(
            DoubleTy,
            {PtrTy},
            false
        );
        ShadowLoadFTy = llvm::FunctionType::get(
            FloatTy,
            {PtrTy},
            false
        );

        Printf = M.getOrInsertFunction("printf", PrintfTy);
        
        ShadowStoreF = M.getOrInsertFunction("shadow_store_float", ShadowStoreFTy);
        ShadowStoreD = M.getOrInsertFunction("shadow_store_double", ShadowStoreDTy);
        ShadowLoadF = M.getOrInsertFunction("shadow_load_float", ShadowLoadFTy);
        ShadowLoadD = M.getOrInsertFunction("shadow_load_double", ShadowLoadDTy);

    }
}