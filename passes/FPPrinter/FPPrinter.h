#ifndef LLVM_PASS_FPPrinter_H
#define LLVM_PASS_FPPrinter_H

// #include "llvm/ADT/MapVector.h"
// #include "llvm/ADT/SmallPtrSet.h"
// #include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/Dominators.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"


// #include "llvm/ADT/StringMap.h"
// #include "llvm/IR/Function.h"
// #include "llvm/Support/raw_ostream.h"

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
// struct FPP : public llvm::AnalysisInfoMixin<FPP> {
//   using Result = ResultFPP;
//   Run(llvm::Function &F, llvm::FunctionAnalysisManager &);

//   static bool isRequired() { return true; }
// };

class FPPrinter : public llvm::PassInfoMixin<FPPrinter> {
public:
  llvm::PreservedAnalyses run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  static bool isRequired() { return true; }
};
#endif


// struct InjectFPPrinter : public llvm::PassInfoMixin<InjectFPPRinter> {
//   llvm::PreservedAnalyses run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
//   bool runOnModule(llvm::Module &M);
//   static bool isRequired() { return true; }
// }