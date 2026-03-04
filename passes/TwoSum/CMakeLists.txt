add_llvm_pass_plugin(TwoSum
  TwoSum.cpp
)

target_compile_features(TwoSum PRIVATE cxx_std_17)
target_include_directories(TwoSum PRIVATE ${CMAKE_SOURCE_DIR}/include)