#!/usr/bin/bash
# sh scripts/build.sh 0 1 2

PROFILE=${1:-0}
FP_DEBUG=${2:-0}
OPT=${3:-0}

if [ $# -ne 0 ] && [ $# -ne 3 ]; then
    echo "Usage: $0 <PROFILING> <DEBUG> <OPT>"
    exit 1
fi

if [ "$PROFILE" -eq 0 ]; then
    PROFILE_CMAKE=OFF
else
    PROFILE_CMAKE=ON
fi

if [ "$FP_DEBUG" -eq 0 ]; then
    FP_DEBUG_CMAKE=OFF
else
    FP_DEBUG_CMAKE=ON
fi

cmake -S . -B build \
    -DLLVM_DIR="$(llvm-config --cmakedir)" \
    -DENABLE_PROFILE="$PROFILE_CMAKE" \
    -DENABLE_FP_DEBUG="$FP_DEBUG_CMAKE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS_RELEASE="-O$OPT -DNDEBUG" \
    -DCMAKE_CXX_FLAGS_RELEASE="-O$OPT -DNDEBUG"

cmake --build build