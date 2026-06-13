#ifndef LLVM_PASS_ShadowMem_H
#define LLVM_PASS_ShadowMem_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

class ShadowMemPass : public llvm::PassInfoMixin<ShadowMemPass> {
public:
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);
    static bool isRequired() { return true; }
};
#endif