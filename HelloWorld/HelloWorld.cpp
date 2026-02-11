#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// keep everything in an anonymous namespace
namespace {
    void visitor(Function &F) {
        errs() << "(llvm-pass) Hello from: " << F.getName() << "\n" << "\n";
        errs() << "(llvm-pass)   number of arguments: " << F.arg_size() << "\n" << "\n";
    }

    // New PM implementation
    struct HelloWorld : PassInfoMixin<HelloWorld> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            visitor(F);
            return PreservedAnalyses::all();
        }
    static bool isRequired() {
        return true;
    }
};
}

// New PM Registration
llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "HelloWorld", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                        ArrayRef<PassBuilder::PipelineElement>) {
                            if (Name == "hello-world") {
                                FPM.addPass(HelloWorld());
                                return true;
                            }
                            return false;
                        });
            }};
}

// Core Interface for pass plugins
// opt will be able to recognize HelloWorld when added to pass pipeline
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getHelloWorldPluginInfo();
}