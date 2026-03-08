cmake_minimum_required(VERSION 3.20)
project(llvm_pass LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${CMAKE_SOURCE_DIR}/cmake")
include(AddLLVM)
include(LLVMPass)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(runtime)
add_subdirectory(passes)