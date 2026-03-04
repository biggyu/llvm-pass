#include "FPPrinter.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "print<fp>-injected"

void visitor(Function &F) {
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      for (int i = 0; i < I.getNumOperands(); i++) {
        llvm::Value* operand = I.getOperand(i);
        if (operand->getType()->isFloatTy()) {
          errs() << "(llvm-pass) Float type detected in " << F.getName() << "\n";
          operand->print(errs());
          errs() << " (type = ";
          operand->getType()->print(errs());
          errs() << ") \n";
        }
        else if (operand->getType()->isDoubleTy()) {
          errs() << "(llvm-pass) Double type detected in " << F.getName() << "\n";
          operand->print(errs());
          errs() << " (type = ";
          operand->getType()->print(errs());
          errs() << ") \n";
        }
        else if (operand->getType()->isIntegerTy()) {
          errs() << "(llvm-pass) Integer type detected in " << F.getName() << "\n";
          operand->print(errs());
          errs() << " (type = ";
          operand->getType()->print(errs());
          errs() << ") \n";
        }
      }
    }
  }
}

void runOnModule(llvm::Module &M) {
  auto &CTX = M.getContext();
  PointerType *PrintfArgTy = PointerType::getUnqual(CTX);

  FunctionType *PrintfTy = FunctionType::get(
    IntegerType::getInt32Ty(CTX),
    PrintfArgTy,
    true);
  
  FunctionCallee Printf = M.getOrInsertFunction("printf", PrintfTy);

  // Function *PrintfF = dyn_cast<Function>(Printf.getCallee());
  // PrintfF->setDoesNotThrow();
  // PrintfF->addParamAttr(0, llvm::Attribute::getWithCaptureInfo(M.getContext(), llvm::CaptureInfo::none()));
  // PrintfF->addParamAttr(0, Attribute::ReadOnly);

  IRBuilder<> GlobalB(CTX);
  // llvm::Constant *FloatFormatConst = llvm::ConstantDataArray::getString(
  //   CTX, "(llvm-pass) Float type detected with value %f\n");
  // llvm::GlobalVariable *FloatFormatStr = new llvm::GlobalVariable(
  //   M, FloatFormatConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, FloatFormatConst, "formatStr");
  llvm::Value *FloatFormatPtr = 
  GlobalB.CreateGlobalStringPtr("(llvm-pass) Float type detected with value %f\n",
                                "fmt.float", 0, &M);
  llvm::Value *IntegerFormatPtr = 
  GlobalB.CreateGlobalStringPtr("(llvm-pass) Integer type detected with value %d\n",
                                "fmt.int", 0, &M);

  // llvm::Constant *DoubleFormatConst = llvm::ConstantDataArray::getString(
  //   CTX, "(llvm-pass) Double type detected with value %lf\n");
  // llvm::GlobalVariable *DoubleFormatStr = new llvm::GlobalVariable(
  //   M, DoubleFormatConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, DoubleFormatConst, "formatStr");
  // llvm::Constant *IntegerFormatConst = llvm::ConstantDataArray::getString(
  //   CTX, "(llvm-pass) Integer type detected with value %f\n");
  // llvm::GlobalVariable *IntegerFormatStr = new llvm::GlobalVariable(
  //   M, IntegerFormatConst->getType(), true, llvm::GlobalVariable::PrivateLinkage, IntegerFormatConst, "formatStr");
    
  // Constant *PrintFloatFormatVar = M.getOrInsertGlobal("PrintFloatFormat", PrintFloatFormat->getType());
  // dyn_cast<GlobalVariable>(PrintFloatFormatVar)->setInitializer(PrintFloatFormat);
  // Constant *PrintDoubleFormatVar = M.getOrInsertGlobal("PrintDoubleFormat", PrintDoubleFormat->getType());
  // dyn_cast<GlobalVariable>(PrintDoubleFormatVar)->setInitializer(PrintDoubleFormat);
  // Constant *PrintIntegerFormatVar = M.getOrInsertGlobal("PrintIntegerFormat", PrintIntegerFormat->getType());
  // dyn_cast<GlobalVariable>(PrintIntegerFormatVar)->setInitializer(PrintIntegerFormat);

  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    for (BasicBlock &BB : F) {
      for(Instruction &I : BB) {
        if (isa<PHINode>(&I)) continue;
        IRBuilder<> Builder(&I);
        for (int i = 0; i < I.getNumOperands(); i++) {
          llvm::Value* operand = I.getOperand(i);
          if (operand->getType()->isFloatTy() || operand->getType()->isDoubleTy() ) {
            llvm::Value *AsDouble = Builder.CreateFPExt(operand, Type::getDoubleTy(CTX));
            Builder.CreateCall(Printf, {FloatFormatPtr, AsDouble});
          }
          // else if (operand->getType()->isDoubleTy()) {
          //   errs() << "(llvm-pass) Double type detected in " << F.getName() << "\n";
          //   operand->print(errs());
          //   errs() << " (type = ";
          //   operand->getType()->print(errs());
          //   errs() << ") \n";
          // }
          else if (operand->getType()->isIntegerTy()) {
            Builder.CreateCall(Printf, {IntegerFormatPtr, operand});
            // errs() << "(llvm-pass) Integer type detected in " << F.getName() << "\n";
            // operand->print(errs());
            // errs() << " (type = ";
            // operand->getType()->print(errs());
            // errs() << ") \n";
          }
        }
      }
    }
  }
}

PreservedAnalyses FPPrinter::run(Function &F, FunctionAnalysisManager &) {
  visitor(F);
  return PreservedAnalyses::all();
}

PreservedAnalyses FPPrinter::run(Module &M, ModuleAnalysisManager &) {
  runOnModule(M);
  return PreservedAnalyses::all();
  // bool Changed = runOnModule(M);
  // return (Changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all());
}

llvm::PassPluginLibraryInfo getFPPrinterPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FPPrinter", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
              [&](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "print<fp>") {
                  FPM.addPass(FPPrinter());
                  return true;
                }
                return false;
              });
            PB.registerPipelineParsingCallback(
              [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "print<fp>-injected") {
                  MPM.addPass(FPPrinter());
                  return true;
                }
                return false;
              });
          }};
}

// llvm::PassPluginLibraryInfo getInjectFPPrinterPluginInfo() {
//   return {LLVM_PLUGIN_API_VERSION, "inject-fp-printer", LLVM_VERSION_STRING,
//           [](PassBuilder &PB) {
//             PB.registerPipelineParsingCallback(
//               [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
//                 if (Name == "print<inject-fp>") {
//                   MPM.addPass(InjectFPPrinter());
//                   return true;
//                 }
//                 return fasle;
//               }
//             );
//           }
//   };
// }

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getFPPrinterPluginInfo();
}
