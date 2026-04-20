#!/usr/bin/bash
# sh scripts/build.sh 0 2

PROFILE=${1:-0}
OPT_FLAG=${2:-0}

if [ $# -ne 0 ] && [ $# -ne 2 ]; then
    echo "Usage: $0 <PROFILING> <OPT>"
    exit 1
fi

if [ "$PROFILE" -eq 0 ]; then
  cmake -S . -B build \
    -DLLVM_DIR="$(llvm-config --cmakedir)" \
    -DENABLE_RUNTIME_TIME=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG"
else
  cmake -S . -B build \
    -DLLVM_DIR="$(llvm-config --cmakedir)" \
    -DENABLE_RUNTIME_TIME=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG"
fi

cmake --build build