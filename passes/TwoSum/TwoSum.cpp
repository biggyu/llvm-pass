#include "TwoSum.h"
// #include "TwoSum_rt.cpp"
#include <cstring>
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


void runOnModule(llvm::Module &M) {
  auto &Ctx = M.getContext();
  
  Type* fretTy = Type::getFloatTy(Ctx);
  Type* dretTy = Type::getDoubleTy(Ctx);
  Type* TSretTy = Type::getVoidTy(Ctx);
  
  PointerType *PrintfArgTy = PointerType::getUnqual(Ctx);
  std::vector<Type*> fparamTy = {Type::getFloatTy(Ctx), Type::getFloatTy(Ctx)};
  std::vector<Type*> dparamTy = {Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx)};
  std::vector<Type*> TSDparamTy = {Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx), PointerType::getUnqual(Ctx), PointerType::getUnqual(Ctx)};
  // std::vector<Type*> TSFparamTy = {Type::getFloatTy(Ctx), Type::getDoubleTy(Ctx), PointerType::getUnqual(Ctx), PointerType::getUnqual(Ctx)};
  std::vector<Type*> TPDparamTy = {Type::getDoubleTy(Ctx), Type::getDoubleTy(Ctx), PointerType::getUnqual(Ctx), PointerType::getUnqual(Ctx)};

  FunctionType *PrintfTy = FunctionType::get(
    IntegerType::getInt32Ty(Ctx),
    PrintfArgTy,
    true);
  FunctionType *FAddTy = FunctionType::get(
    fretTy,
    fparamTy,
    false);
  FunctionType *DAddTy = FunctionType::get(
    dretTy,
    dparamTy,
    false);
  FunctionType *TwoSumDTy = FunctionType::get(
    TSretTy,
    TSDparamTy,
    false);
  // FunctionType *TwoSumFTy = FunctionType::get(
  //   TSretTy,
  //   TSFparamTy,
  //   false);
  FunctionType *TwoProdDTy = FunctionType::get(
    TSretTy,
    TPDparamTy,
    false);
  
  FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);
  FunctionCallee addf = M.getOrInsertFunction("addf", FAddTy);
  FunctionCallee addd = M.getOrInsertFunction("addd", DAddTy);
  FunctionCallee TwoSumD = M.getOrInsertFunction("TwoSum", TwoSumDTy);
  // FunctionCallee TwoSumF = M.getOrInsertFunction("TwoSum_F", TwoSumFTy);
  FunctionCallee TwoProdD = M.getOrInsertFunction("TwoProd", TwoProdDTy);

  IRBuilder<> GlobalB(Ctx);
  
  Value *TwoSumFormatPtr = 
  GlobalB.CreateGlobalStringPtr("(llvm-pass) TwoSum result\nx = %f dx = %f\n",
                                "fmt.float", 0, &M);
  Value *TwoProdFormatPtr = 
  GlobalB.CreateGlobalStringPtr("(llvm-pass) TwoProd result\nx = %f dx = %f\n",
                                "fmt.float", 0, &M);
  // Value *FloatFormatPtr = GlobalB.CreateGlobalStringPtr("(llvm-pass) TwoSum result:\nx=%f dx=%f\n");

  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    if (F.getName() == "addf" || F.getName() == "addd" || F.getName() == "TwoSum" || F.getName() == "TwoProd") continue;

    for (BasicBlock &BB : F) {
      for(Instruction &I : BB) {
        if (isa<PHINode>(&I)) continue;

        IRBuilder<> Builder(&I);
        
        if (I.getOpcode() == Instruction::FAdd) {
          // if (I.getOperand(0)->getType()->isFloatTy()) {
          //   Value *opr0 = I.getOperand(0);
          //   Value *opr1 = I.getOperand(1);
          //   AllocaInst *x = Builder.CreateAlloca(Type::getFloatTy(Ctx), nullptr, "twosum.x");
          //   AllocaInst *dx = Builder.CreateAlloca(Type::getFloatTy(Ctx), nullptr, "twosum.dx");
          //   // llvm::CallInst *call = Builder.CreateCall(addf, {opr0, opr1}, "ret_addf");
          //   Builder.CreateCall(TwoSumF, {opr0, opr1, x, dx});
          //   Value *xval = Builder.CreateLoad(Type::getFloatTy(Ctx), x, "twosum.xval");
          //   Value *dxval = Builder.CreateLoad(Type::getFloatTy(Ctx), dx, "twosum.dxval");
          //   llvm::Value *xAsFloat = Builder.CreateFPExt(xval, Type::getDoubleTy(Ctx));
          //   llvm::Value *dxAsFloat = Builder.CreateFPExt(dxval, Type::getDoubleTy(Ctx));
          //   Builder.CreateCall(Printf, {FloatFormatPtr, xAsFloat, dxAsFloat});
          // }
          // else if (I.getOperand(0)->getType()->isDoubleTy()) {
          if (I.getOperand(0)->getType()->isDoubleTy()) {
            Value *opr0 = I.getOperand(0);
            Value *opr1 = I.getOperand(1);
            AllocaInst *x = Builder.CreateAlloca(dretTy, nullptr, "twosum.x");
            AllocaInst *dx = Builder.CreateAlloca(dretTy, nullptr, "twosum.dx");
            // llvm::CallInst *call = Builder.CreateCall(addd, {opr0, opr1}, "ret_addd");
            Builder.CreateCall(TwoSumD, {opr0, opr1, x, dx});
            Value *xval = Builder.CreateLoad(dretTy, x, "twosum.xval");
            Value *dxval = Builder.CreateLoad(dretTy, dx, "twosum.dxval");
            // Value *FloatFormatPtr = GlobalB.CreateGlobalStringPtr("(llvm-pass) TwoSum result:\nx=%f dx=%f\n");
            Builder.CreateCall(Printf, {TwoSumFormatPtr, xval, dxval});
          }
        }
        else if (I.getOpcode() == Instruction::FSub) {
          if (I.getOperand(0)->getType()->isDoubleTy()) {
            Value *opr0 = I.getOperand(0);
            Value *opr1 = I.getOperand(1);
            AllocaInst *x = Builder.CreateAlloca(dretTy, nullptr, "twosum.x");
            AllocaInst *dx = Builder.CreateAlloca(dretTy, nullptr, "twosum.dx");
            Value *invopr1 = Builder.CreateFNeg(opr1, "inv");
            // llvm::CallInst *call = Builder.CreateCall(addd, {opr0, opr1}, "ret_addd");
            Builder.CreateCall(TwoSumD, {opr0, invopr1, x, dx});
            Value *xval = Builder.CreateLoad(dretTy, x, "twosum.xval");
            Value *dxval = Builder.CreateLoad(dretTy, dx, "twosum.dxval");
            // Value *FloatFormatPtr = GlobalB.CreateGlobalStringPtr("(llvm-pass) TwoSum result:\nx=%f dx=%f\n");
            Builder.CreateCall(Printf, {TwoSumFormatPtr, xval, dxval});
          }
        }
        else if (I.getOpcode() == Instruction::FMul) {
          if (I.getOperand(0)->getType()->isDoubleTy()) {
            Value *opr0 = I.getOperand(0);
            Value *opr1 = I.getOperand(1);
            AllocaInst *x = Builder.CreateAlloca(Type::getDoubleTy(Ctx), nullptr, "twoprod.x");
            AllocaInst *dx = Builder.CreateAlloca(Type::getDoubleTy(Ctx), nullptr, "twoprod.dx");
            Builder.CreateCall(TwoProdD, {opr0, opr1, x, dx});
            Value *xval = Builder.CreateLoad(dretTy, x, "twoprod.xval");
            Value *dxval = Builder.CreateLoad(dretTy, dx, "twoprod.dxval");
            Builder.CreateCall(Printf, {TwoProdFormatPtr, xval, dxval});
          }
        }
        
        // if (I.getOpcode() == Instruction::FAdd) {
        //   if (I.getOperand(0)->getType()->isFloatTy()) {
        //     Value *opr0 = I.getOperand(0);
        //     Value *opr1 = I.getOperand(1);
        //     llvm::CallInst *call = Builder.CreateCall(addf, {opr0, opr1}, "ret_addf");
        //     llvm::Value *callAsFloat = Builder.CreateFPExt(call, Type::getDoubleTy(Ctx));
        //     Builder.CreateCall(Printf, {FloatFormatPtr, callAsFloat});
        //   }
        //   else if (I.getOperand(0)->getType()->isDoubleTy()) {
        //     Value *opr0 = I.getOperand(0);
        //     Value *opr1 = I.getOperand(1);
        //     llvm::CallInst *call = Builder.CreateCall(addd, {opr0, opr1}, "ret_addd");
        //     Builder.CreateCall(Printf, {FloatFormatPtr, call});
        //   }
        // }
      }
    }
  }
}

PreservedAnalyses TwoSumPass::run(Module &M, ModuleAnalysisManager &) {
  runOnModule(M);
  return PreservedAnalyses::all();
  // bool Changed = runOnModule(M);
  // return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

llvm::PassPluginLibraryInfo getTwoSumPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "TwoSumPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
              [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "twosum") {
                  MPM.addPass(TwoSumPass());
                  return true;
                }
                return false;
              });
          }};
}


extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getTwoSumPluginInfo();
}
