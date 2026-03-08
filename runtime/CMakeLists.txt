add_library(fp_runtime STATIC fp_runtime.cpp)
target_include_directories(fp_runtime PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(fp_runtime PUBLIC cxx_std_17)