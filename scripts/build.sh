#!/usr/bin/bash
# sh scripts/build.sh 0 1 2

RUNTIME_TIME=${1:-0}
FP_DEBUG=${2:-0}
OPT_FLAG=${3:-0}

if [ $# -ne 0 ] && [ $# -ne 3 ]; then
    echo "Usage: $0 <RUNTIME_TIME> <FP_DEBUG> <OPT>"
    exit 1
fi

if [ "$RUNTIME_TIME" -eq 0 ]; then
    RUNTIME_TIME_CMAKE=OFF
else
    RUNTIME_TIME_CMAKE=ON
fi

if [ "$FP_DEBUG" -eq 0 ]; then
    FP_DEBUG_CMAKE=OFF
else
    FP_DEBUG_CMAKE=ON
fi

cmake -S . -B build \
    -DLLVM_DIR="$(llvm-config --cmakedir)" \
    -DENABLE_RUNTIME_TIME="$RUNTIME_TIME_CMAKE" \
    -DENABLE_FP_DEBUG="$FP_DEBUG_CMAKE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O$OPT_FLAG -DNDEBUG"

cmake --build build