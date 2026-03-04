add_llvm_pass_plugin(FPPrinter
  FPPrinter.cpp
)

target_compile_features(FPPrinter PRIVATE cxx_std_17)
target_include_directories(FPPrinter PRIVATE ${CMAKE_SOURCE_DIR}/include)