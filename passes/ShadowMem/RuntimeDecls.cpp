#include "decls_fp.h"

using namespace llvm;

namespace utils {
    RuntimeFns::RuntimeFns(Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        BoolTy = llvm::Type::getInt1Ty(Ctx);
        TrueVal = llvm::ConstantInt::getTrue(Ctx);
        FalseVal = llvm::ConstantInt::getFalse(Ctx);
        I32Ty = llvm::Type::getInt32Ty(Ctx);
        I64Ty = llvm::Type::getInt64Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);
        FnPtrTy = llvm::PointerType::get(Ctx, 0);
        
        ShadowEntryTy = llvm::StructType::create(Ctx, "ShadowEntry");
        ShadowEntryTy->setBody({I64Ty, DoubleTy, DoubleTy, BoolTy, DoubleTy, BoolTy, DoubleTy, BoolTy}, false);

        ZeroD = llvm::ConstantFP::get(DoubleTy, 0.0);
        ZeroF = llvm::ConstantFP::get(FloatTy, 0.0);

        PrintfTy = llvm::FunctionType::get(
            I32Ty,
            {PtrTy},
            true
        );

        ShadowStoreDTy = llvm::FunctionType::get(
            VoidTy,
            {PtrTy, DoubleTy, DoubleTy, BoolTy, DoubleTy, BoolTy, DoubleTy},
            false
        );
        ShadowStoreFTy = llvm::FunctionType::get(
            VoidTy,
            {PtrTy, FloatTy, DoubleTy, BoolTy, DoubleTy, BoolTy, DoubleTy},
            false
        );
        ShadowLoadDTy = llvm::FunctionType::get(
            ShadowEntryTy,
            {PtrTy, DoubleTy},
            false
        );
        ShadowLoadFTy = llvm::FunctionType::get(
            ShadowEntryTy,
            {PtrTy, FloatTy},
            false
        );
        ShadowStackPushTy = llvm::FunctionType::get(
            VoidTy,
            {DoubleTy, DoubleTy, BoolTy, DoubleTy, BoolTy, DoubleTy},
            false
        );
        ShadowStackPopTy = llvm::FunctionType::get(
            ShadowEntryTy,
            {},
            false
        );

        CheckConvSITy = llvm::FunctionType::get(
            VoidTy,
            {I32Ty, DoubleTy, DoubleTy, I32Ty},
            false
        );
        CheckConvUITy = llvm::FunctionType::get(
            VoidTy,
            {I64Ty, DoubleTy, DoubleTy, I32Ty},
            false
        );
        CheckBranchTy = llvm::FunctionType::get(
            VoidTy,
            {DoubleTy, DoubleTy, DoubleTy, DoubleTy, I64Ty, BoolTy, I32Ty},
            false
        );

        CheckErrorTy = llvm::FunctionType::get(
            VoidTy,
            {DoubleTy, DoubleTy, I32Ty, I32Ty},
            false
        );

        AtexitTy = llvm::FunctionType::get(
            I32Ty,
            {FnPtrTy},
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

        ConditionNumberTy = llvm::FunctionType::get(
            DoubleTy,
            {I32Ty, DoubleTy, DoubleTy, DoubleTy, DoubleTy, BoolTy, BoolTy, I32Ty},
            false
        );
        // ConditionNumberDTy = llvm::FunctionType::get(
        //     VoidTy,
        //     {I32Ty, DoubleTy, DoubleTy, DoubleTy, DoubleTy, BoolTy, BoolTy, I32Ty},
        //     false
        // );
        // ConditionNumberFTy = llvm::FunctionType::get(
        //     VoidTy,
        //     {I32Ty, FloatTy, FloatTy, FloatTy, FloatTy, BoolTy, BoolTy, I32Ty},
        //     false
        // );

        Printf = M.getOrInsertFunction("printf", PrintfTy);
        
        ShadowStoreF = M.getOrInsertFunction("shadow_store_float", ShadowStoreFTy);
        ShadowStoreD = M.getOrInsertFunction("shadow_store_double", ShadowStoreDTy);
        ShadowLoadF = M.getOrInsertFunction("shadow_load_float", ShadowLoadFTy);
        ShadowLoadD = M.getOrInsertFunction("shadow_load_double", ShadowLoadDTy);
        ShadowStackPush = M.getOrInsertFunction("shadow_stack_push", ShadowStackPushTy);
        ShadowStackPop = M.getOrInsertFunction("shadow_stack_pop", ShadowStackPopTy);

        CheckConvSI = M.getOrInsertFunction("check_conv_si", CheckConvSITy);
        CheckConvUI = M.getOrInsertFunction("check_conv_ui", CheckConvUITy);
        CheckBranch = M.getOrInsertFunction("check_branch", CheckBranchTy);
        CheckError = M.getOrInsertFunction("check_error", CheckErrorTy);
        Atexit = M.getOrInsertFunction("atexit", AtexitTy);
        RegisterFPSite = M.getOrInsertFunction("register_fp_site", RegisterFPSiteTy);
        ReportDebugSummary = M.getOrInsertFunction("report_debug_summary", ReportDebugSummaryTy);
        // ConditionNumberF = M.getOrInsertFunction("condition_number_float", ConditionNumberFTy);
        // ConditionNumberD = M.getOrInsertFunction("condition_number_double", ConditionNumberDTy);
        ConditionNumber = M.getOrInsertFunction("condition_number", ConditionNumberTy);
    }
}