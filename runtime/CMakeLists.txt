add_library(pass_runtime STATIC 
    fp_condition.cpp
    fp_debug.cpp
    mpfr_runtime.cpp
    smem_runtime.cpp
)

if (ENABLE_PROFILE)
    target_compile_definitions(pass_runtime PRIVATE ENABLE_PROFILE=1)
endif()

if (ENABLE_FP_DEBUG)
    target_compile_definitions(pass_runtime PRIVATE ENABLE_FP_DEBUG=1)
endif()

target_include_directories(pass_runtime PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(pass_runtime PUBLIC cxx_std_17)
