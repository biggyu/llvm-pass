#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"

namespace utils {
    struct RuntimeFns {
        llvm::Module &M;
        llvm::LLVMContext &Ctx;

        llvm::Type *VoidTy;
        llvm::Type *I32Ty;
        llvm::Type *FloatTy;
        llvm::Type *DoubleTy;
        llvm::Type *PtrTy;

        llvm::Constant *ZeroF;
        llvm::Constant *ZeroD;

        llvm::FunctionCallee Printf;
        llvm::FunctionCallee PropSumFError;
        llvm::FunctionCallee PropSumDError;
        llvm::FunctionCallee PropProdFError;
        llvm::FunctionCallee PropProdDError;
        // llvm::FunctionCallee TwoSumF;
        // llvm::FunctionCallee TwoSumD;
        // llvm::FunctionCallee TwoProdF;
        // llvm::FunctionCallee TwoProdD;
        llvm::FunctionCallee ShadowStoreD;
        llvm::FunctionCallee ShadowStoreF;
        llvm::FunctionCallee ShadowLoadD;
        llvm::FunctionCallee ShadowLoadF;


        llvm::FunctionType *PrintfTy;
        llvm::FunctionType *PropSumFErrorTy;
        llvm::FunctionType *PropSumDErrorTy;
        llvm::FunctionType *PropProdFErrorTy;
        llvm::FunctionType *PropProdDErrorTy;
        // llvm::FunctionType *TwoSumFTy;
        // llvm::FunctionType *TwoSumDTy;
        // llvm::FunctionType *TwoProdFTy;
        // llvm::FunctionType *TwoProdDTy;
        llvm::FunctionType *ShadowStoreFTy;
        llvm::FunctionType *ShadowStoreDTy;
        llvm::FunctionType *ShadowLoadFTy;
        llvm::FunctionType *ShadowLoadDTy;

        explicit RuntimeFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
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
            PropSumFErrorTy = llvm::FunctionType::get(
                VoidTy,
                {FloatTy, FloatTy, FloatTy, FloatTy, PtrTy, PtrTy},
                false
            );
            PropSumDErrorTy = llvm::FunctionType::get(
                VoidTy,
                {DoubleTy, DoubleTy, DoubleTy, DoubleTy, PtrTy, PtrTy},
                false
            );
            PropProdFErrorTy = llvm::FunctionType::get(
                VoidTy,
                {FloatTy, FloatTy, FloatTy, FloatTy, PtrTy, PtrTy},
                false
            );
            PropProdDErrorTy = llvm::FunctionType::get(
                VoidTy,
                {DoubleTy, DoubleTy, DoubleTy, DoubleTy, PtrTy, PtrTy},
                false
            );
            // TwoSumFTy = llvm::FunctionType::get(
            //     VoidTy,
            //     {FloatTy, FloatTy, PtrTy, PtrTy},
            //     false
            // );
            // TwoSumDTy = llvm::FunctionType::get(
            //     VoidTy,
            //     {DoubleTy, DoubleTy, PtrTy, PtrTy},
            //     false
            // );
            // TwoProdFTy = llvm::FunctionType::get(
            //     VoidTy,
            //     {FloatTy, FloatTy, PtrTy, PtrTy},
            //     false
            // );
            // TwoProdDTy = llvm::FunctionType::get(
            //     VoidTy,
            //     {DoubleTy, DoubleTy, PtrTy, PtrTy},
            //     false
            // );
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
            PropSumFError = M.getOrInsertFunction("PropSumFError", PropSumFErrorTy);
            PropSumDError = M.getOrInsertFunction("PropSumDError", PropSumDErrorTy);
            PropProdFError = M.getOrInsertFunction("PropProdFError", PropProdFErrorTy);
            PropProdDError = M.getOrInsertFunction("PropProdDError", PropProdDErrorTy);
            // TwoSumF = M.getOrInsertFunction("TwoSumF", TwoSumFTy);
            // TwoSumD = M.getOrInsertFunction("TwoSumD", TwoSumDTy);
            // TwoProdF = M.getOrInsertFunction("TwoProdF", TwoProdFTy);
            // TwoProdD = M.getOrInsertFunction("TwoProdD", TwoProdDTy);
            ShadowStoreF = M.getOrInsertFunction("shadow_store_float", ShadowStoreFTy);
            ShadowStoreD = M.getOrInsertFunction("shadow_store_double", ShadowStoreDTy);
            ShadowLoadF = M.getOrInsertFunction("shadow_load_float", ShadowLoadFTy);
            ShadowLoadD = M.getOrInsertFunction("shadow_load_double", ShadowLoadDTy);

            // llvm::IRBuilder<> GlobalB(Ctx);
            // llvm::Value *TwoSumFormatPtr = GlobalB.CreateGlobalStringPtr(
            //     "(llvm-pass) TwoSum result\nx = %f dx = %f\n", "fmt.float", 0, &M);
            // llvm::Value *TwoProdFormatPtr = GlobalB.CreateGlobalStringPtr(
            //     "(llvm-pass) TwoProd result\nx = %f dx = %f\n", "fmt.float", 0, &M);
        }

    };
}