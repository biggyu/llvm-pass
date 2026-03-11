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

        llvm::FunctionCallee Printf;
        llvm::FunctionCallee TwoSum;
        llvm::FunctionCallee TwoProd;

        llvm::FunctionType *PrintfTy;
        llvm::FunctionType *TwoSumTy;
        llvm::FunctionType *TwoProdTy;

        explicit RuntimeFns(llvm::Module &Mod) : M(Mod), Ctx(Mod.getContext()) {
            VoidTy = llvm::Type::getVoidTy(Ctx);
            I32Ty = llvm::Type::getInt32Ty(Ctx);
            FloatTy = llvm::Type::getFloatTy(Ctx);
            DoubleTy = llvm::Type::getDoubleTy(Ctx);
            PtrTy = llvm::PointerType::getUnqual(Ctx);

            PrintfTy = llvm::FunctionType::get(
                I32Ty,
                {PtrTy},
                true
            );
            TwoSumTy = llvm::FunctionType::get(
                VoidTy,
                {DoubleTy, DoubleTy, PtrTy, PtrTy},
                false
            );
            TwoProdTy = llvm::FunctionType::get(
                VoidTy,
                {DoubleTy, DoubleTy, PtrTy, PtrTy},
                false
            );

            Printf = M.getOrInsertFunction("printf", PrintfTy);
            TwoSum = M.getOrInsertFunction("TwoSum", TwoSumTy);
            TwoProd = M.getOrInsertFunction("TwoProd", TwoProdTy);


            // llvm::IRBuilder<> GlobalB(Ctx);
            // llvm::Value *TwoSumFormatPtr = GlobalB.CreateGlobalStringPtr(
            //     "(llvm-pass) TwoSum result\nx = %f dx = %f\n", "fmt.float", 0, &M);
            // llvm::Value *TwoProdFormatPtr = GlobalB.CreateGlobalStringPtr(
            //     "(llvm-pass) TwoProd result\nx = %f dx = %f\n", "fmt.float", 0, &M);
        }

    };
}