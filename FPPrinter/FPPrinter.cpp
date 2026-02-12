#include "FPPrinter.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
// #include "llvm/IR/Constants.h"
// #include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void visitor(Function &F) {
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      for (int i = 0; i < I.getNumOperands(); i++) {
        llvm::Value* operand = I.getOperand(i);
        if (operand->getType()->isFloatTy() || operand->getType()->isDoubleTy()) {
          errs() << "(llvm-pass) This is a floating point inside " << F.getName() << "\n";
          operand->print(errs());
          errs() << " (type = ";
          operand->getType()->print(errs());
          errs() << ") \n";
        }
      }
    }
  }
}

PreservedAnalyses FPPrinter::run(Function &F, FunctionAnalysisManager &) {
  visitor(F);
  return PreservedAnalyses::all();
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
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getFPPrinterPluginInfo();
}