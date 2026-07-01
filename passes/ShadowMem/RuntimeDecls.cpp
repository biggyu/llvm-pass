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
            {PtrTy, FloatTy, DoubleTy},
            false
        );
        ShadowLoadDTy = llvm::FunctionType::get(
            DoubleTy,
            {PtrTy, DoubleTy},
            false
        );
        ShadowLoadFTy = llvm::FunctionType::get(
            DoubleTy,
            {PtrTy, FloatTy},
            false
        );

        CheckErrorDTy = llvm::FunctionType::get(
            VoidTy,
            {DoubleTy, DoubleTy, I32Ty, I32Ty},
            false
        );
        CheckErrorFTy = llvm::FunctionType::get(
            VoidTy,
            {FloatTy, DoubleTy, I32Ty, I32Ty},
            false
        );

        RegisterFPSiteTy = llvm::FunctionType::get(
            VoidTy,
            {I32Ty, PtrTy, PtrTy, I32Ty, I32Ty, PtrTy},
            false
        );

        ReportDebugSummaryTy = llvm::FunctionType::get(
            VoidTy,
            {},
            false
        );

        Printf = M.getOrInsertFunction("printf", PrintfTy);
        
        ShadowStoreF = M.getOrInsertFunction("shadow_store_float", ShadowStoreFTy);
        ShadowStoreD = M.getOrInsertFunction("shadow_store_double", ShadowStoreDTy);
        ShadowLoadF = M.getOrInsertFunction("shadow_load_float", ShadowLoadFTy);
        ShadowLoadD = M.getOrInsertFunction("shadow_load_double", ShadowLoadDTy);

        CheckErrorF = M.getOrInsertFunction("check_error_float", CheckErrorFTy);
        CheckErrorD = M.getOrInsertFunction("check_error_double", CheckErrorDTy);
        RegisterFPSite = M.getOrInsertFunction("register_fp_site", RegisterFPSiteTy);
        ReportDebugSummary = M.getOrInsertFunction("report_debug_summary", ReportDebugSummaryTy);
    }
}