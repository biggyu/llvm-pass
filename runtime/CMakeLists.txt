add_library(pass_runtime STATIC 
    # fp_runtime.cpp
    smem_runtime.cpp
    mpfr_runtime.cpp
)
target_include_directories(pass_runtime PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(pass_runtime PUBLIC cxx_std_17)
if(ENABLE_RUNTIME_TIME)
    target_compile_definitions(pass_runtime PRIVATE ENABLE_RUNTIME_TIME=1)
endif()