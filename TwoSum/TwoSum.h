#ifndef LLVM_PASS_TwoSum_H
#define LLVM_PASS_TwoSum_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"


class TwoSumPass : public llvm::PassInfoMixin<TwoSumPass> {
public:
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
  static bool isRequired() { return true; }
};
#endif
