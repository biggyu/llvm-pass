#!/usr/bin/bash
# sh scripts/instruments_lnk.sh ShadowMem shadowmem examples/input_for_matmul.c 0 0 2

PASS=$1
PLUGIN=$2
SRC=$3
PROFILE=${4:-0}
FP_DEBUG=${5:-0}
OPT=${6:-0}

if [ $# -ne 6 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT> <PROFILE> <DEBUG> <OPT>"
    exit 1
fi

RUNTIME_DEFS=""
if [ "$PROFILE" -eq 1 ]; then
    RUNTIME_DEFS="${RUNTIME_DEFS}-DENABLE_PROFILE=ON "
fi
#TODO: error metric
if [ "$FP_DEBUG" -eq 1 ]; then
    RUNTIME_DEFS="${RUNTIME_DEFS}-DENABLE_FP_DEBUG=ON "
    FP_DEBUG_CMAKE=false
else
    FP_DEBUG_CMAKE=true
fi

OUTDIR="build/out/$PASS"
mkdir -p "$OUTDIR"

# FP_RUNTIME_SRC="runtime/fp_runtime.cpp"
SMEM_RUNTIME_SRC="runtime/smem_runtime.cpp"
MPFR_RUNTIME_SRC="runtime/mpfr_runtime.cpp"
FP_DEBUG_SRC="runtime/fp_debug.cpp"

# 1. Compile input source to LLVM IR
# $LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -ffp-contract=off "$SRC" -o "$OUTDIR/input_O$OPT.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -fno-math-errno -ffp-contract=off "$SRC" -o "$OUTDIR/input_O$OPT.ll"

# 2. Run your LLVM pass
$LLVM_OPT \
    -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
    --passes="$PLUGIN" \
    -fp-debug-checks="$FP_DEBUG_CMAKE" \
    -fp-debug-metric=0 \
    -S "$OUTDIR/input_O$OPT.ll" \
    -o "$OUTDIR/instrumented.ll"

# 3. Compile runtime sources to LLVM IR
# $LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude "$FP_RUNTIME_SRC" -o "$OUTDIR/fp_runtime_O$OPT.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$SMEM_RUNTIME_SRC" -o "$OUTDIR/smem_runtime_O$OPT.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$MPFR_RUNTIME_SRC" -o "$OUTDIR/mpfr_runtime_O$OPT.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$FP_DEBUG_SRC" -o "$OUTDIR/fp_debug_O$OPT.ll"

# 4. Link instrumented IR + runtime IR
$LLVM_LINK \
    "$OUTDIR/instrumented.ll" \
    "$OUTDIR/smem_runtime_O$OPT.ll" \
    "$OUTDIR/mpfr_runtime_O$OPT.ll" \
    "$OUTDIR/fp_debug_O$OPT.ll" \
    -S -o "$OUTDIR/linked_O$OPT.ll"

# 5. Optimize the combined IR
$LLVM_OPT -O"$OPT" \
    -S "$OUTDIR/linked_O$OPT.ll" \
    -o "$OUTDIR/optimized_O$OPT.ll"

# 6. Compile optimized combined IR to object
$LLVM_CLANGXX -O"$OPT" \
    -c "$OUTDIR/optimized_O$OPT.ll" \
    -o "$OUTDIR/optimized_O$OPT.o"

# 7. Link final executable
$LLVM_CLANGXX -O"$OPT" \
    "$OUTDIR/optimized_O$OPT.o" \
    -o "$OUTDIR/a.out" \
    -lm -lmpfr -lgmp

# 8. Run
for i in 1 2 3;
do
    "$OUTDIR/a.out"
done
# "$OUTDIR/a.out"