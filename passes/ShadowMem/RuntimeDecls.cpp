#include "decls_fp.h"

using namespace llvm;

namespace utils {
    RuntimeFns::RuntimeFns(Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        BoolTy = llvm::Type::getInt1Ty(Ctx);
        I32Ty = llvm::Type::getInt32Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);
        ShadowEntryTy = llvm::StructType::create(Ctx, "ShadowEntry");
        ShadowEntryTy->setBody({I32Ty, DoubleTy, DoubleTy, BoolTy, BoolTy, DoubleTy, DoubleTy, BoolTy}, false);

        ZeroD = llvm::ConstantFP::get(DoubleTy, 0.0);
        ZeroF = llvm::ConstantFP::get(FloatTy, 0.0);

        PrintfTy = llvm::FunctionType::get(
            I32Ty,
            {PtrTy},
            true
        );

        ShadowStoreTy = llvm::FunctionType::get(
            VoidTy,
            {PtrTy, DoubleTy, DoubleTy, DoubleTy, BoolTy, BoolTy, DoubleTy},
            false
        );
        ShadowLoadTy = llvm::FunctionType::get(
            ShadowEntryTy,
            {PtrTy},
            false
        );

        CheckErrorDTy = llvm::FunctionType::get(
            VoidTy,
            {DoubleTy, DoubleTy, I32Ty, I32Ty},
            false
        );
        CheckErrorFTy = llvm::FunctionType::get(
            VoidTy,
            {FloatTy, FloatTy, I32Ty, I32Ty},
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

        ConditionNumberDTy = llvm::FunctionType::get(
            VoidTy,
            {I32Ty, DoubleTy, DoubleTy, DoubleTy, DoubleTy, BoolTy, BoolTy, I32Ty},
            false
        );
        ConditionNumberFTy = llvm::FunctionType::get(
            VoidTy,
            {I32Ty, FloatTy, FloatTy, FloatTy, FloatTy, BoolTy, BoolTy, I32Ty},
            false
        );

        Printf = M.getOrInsertFunction("printf", PrintfTy);
        
        ShadowStore = M.getOrInsertFunction("shadow_store", ShadowStoreTy);
        ShadowLoad = M.getOrInsertFunction("shadow_load", ShadowLoadTy);

        CheckErrorF = M.getOrInsertFunction("check_error_float", CheckErrorFTy);
        CheckErrorD = M.getOrInsertFunction("check_error_double", CheckErrorDTy);
        RegisterFPSite = M.getOrInsertFunction("register_fp_site", RegisterFPSiteTy);
        ReportDebugSummary = M.getOrInsertFunction("report_debug_summary", ReportDebugSummaryTy);
        ConditionNumberF = M.getOrInsertFunction("condition_number_float", ConditionNumberFTy);
        ConditionNumberD = M.getOrInsertFunction("condition_number_double", ConditionNumberDTy);
    }
}