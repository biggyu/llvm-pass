cmake_minimum_required(VERSION 3.20)
project(FPP)

# LOAD LLVM CONFIGURATION
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm")

find_package(LLVM CONFIG)
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 20)
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 20 or above")
endif()

include_directories((SYSTEM ${LLVM_INCLUDE_DIRS}))

# BUILD CONFIGURATION
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(FPPrinter SHARED FPPrinter.cpp)

target_link_libraries(FPPrinter
    "$<$<PLATFORM_ID:DARWIN>:-undefined dynamic_lookup>")