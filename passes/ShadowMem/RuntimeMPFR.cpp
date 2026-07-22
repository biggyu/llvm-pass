#include "decls_mpfr.h"

using namespace llvm;

namespace utils {
    RuntimeMPFRFns::RuntimeMPFRFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
        VoidTy = llvm::Type::getVoidTy(Ctx);
        // I32Ty = llvm::Type::getInt32Ty(Ctx);
        FloatTy = llvm::Type::getFloatTy(Ctx);
        DoubleTy = llvm::Type::getDoubleTy(Ctx);
        PtrTy = llvm::PointerType::getUnqual(Ctx);
        // FpEntryFTy = llvm::StructType::create(Ctx, "fp_entryF");
        // FpEntryFTy->setBody({FloatTy, FloatTy}, false);
        FpEntryDTy = llvm::StructType::create(Ctx, "fp_entryD");
        FpEntryDTy->setBody({DoubleTy, DoubleTy}, false);

        // PropSinFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropSinDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropCosFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropCosDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropTanFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropTanDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropAsinFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropAsinDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropAcosFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropAcosDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropAtanFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropAtanDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropLogFErrorTy = llvm::FunctionType::get( 
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropLogDErrorTy = llvm::FunctionType::get( 
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );
        
        // PropExpFErrorTy = llvm::FunctionType::get(
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        PropExpDErrorTy = llvm::FunctionType::get(
            FpEntryDTy,
            {DoubleTy, DoubleTy},
            false
        );

        // PropPowFErrorTy = llvm::FunctionType::get(
        //     FpEntryFTy,
        //     {FloatTy, FloatTy, FloatTy, FloatTy},
        //     false
        // );
        PropPowDErrorTy = llvm::FunctionType::get(
            FpEntryDTy,
            {DoubleTy, DoubleTy, DoubleTy, DoubleTy},
            false
        );

        // PropFabsFErrorTy = llvm::FunctionType::get(
        //     FpEntryFTy,
        //     {FloatTy, FloatTy},
        //     false
        // );
        // PropFabsDErrorTy = llvm::FunctionType::get(
        //     FpEntryDTy,
        //     {DoubleTy, DoubleTy},
        //     false
        // );

        // PropSinFError = M.getOrInsertFunction("PropSinFError", PropSinFErrorTy);
        PropSinDError = M.getOrInsertFunction("PropSinDError", PropSinDErrorTy);
        // PropCosFError = M.getOrInsertFunction("PropCosFError", PropCosFErrorTy);
        PropCosDError = M.getOrInsertFunction("PropCosDError", PropCosDErrorTy);
        // PropTanFError = M.getOrInsertFunction("PropTanFError", PropTanFErrorTy);
        PropTanDError = M.getOrInsertFunction("PropTanDError", PropTanDErrorTy);
        // PropAsinFError = M.getOrInsertFunction("PropAsinFError", PropAsinFErrorTy);
        PropAsinDError = M.getOrInsertFunction("PropAsinDError", PropAsinDErrorTy);
        // PropAcosFError = M.getOrInsertFunction("PropAcosFError", PropAcosFErrorTy);
        PropAcosDError = M.getOrInsertFunction("PropAcosDError", PropAcosDErrorTy);
        // PropAtanFError = M.getOrInsertFunction("PropAtanFError", PropAtanFErrorTy);
        PropAtanDError = M.getOrInsertFunction("PropAtanDError", PropAtanDErrorTy);
        // PropLogFError = M.getOrInsertFunction("PropLogFError", PropLogFErrorTy);
        PropLogDError = M.getOrInsertFunction("PropLogDError", PropLogDErrorTy);
        // PropExpFError = M.getOrInsertFunction("PropExpFError", PropExpFErrorTy);
        PropExpDError = M.getOrInsertFunction("PropExpDError", PropExpDErrorTy);
        // PropPowFError = M.getOrInsertFunction("PropPowFError", PropPowFErrorTy);
        PropPowDError = M.getOrInsertFunction("PropPowDError", PropPowDErrorTy);
        // PropFabsFError = M.getOrInsertFunction("PropFabsFError", PropFabsFErrorTy);
        // PropFabsDError = M.getOrInsertFunction("PropFabsDError", PropFabsDErrorTy);
    }
}