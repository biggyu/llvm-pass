add_llvm_pass_plugin(HelloWorld
  HelloWorld.cpp
)

target_compile_features(HelloWorld PRIVATE cxx_std_17)
target_include_directories(HelloWorld PRIVATE ${CMAKE_SOURCE_DIR}/include)