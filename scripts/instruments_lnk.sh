#!/usr/bin/bash
# sh scripts/instruments_lnk.sh ShadowMem shadowmem examples/input_for_matmul.c 2

PASS=$1
PLUGIN=$2
SRC=$3
OPT=$4

if [ $# -ne 4 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT> <OPT>"
    exit 1
fi

OUTDIR="build/out/$PASS"
mkdir -p "$OUTDIR"

# Change these to your actual runtime source files
# FP_RUNTIME_SRC="runtime/fp_runtime.cpp"
SMEM_RUNTIME_SRC="runtime/smem_runtime.cpp"
# MPFR_RUNTIME_SRC="runtime/mpfr_runtime.cpp"

# 1. Compile input source to LLVM IR
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -ffp-contract=off "$SRC" -o "$OUTDIR/input_O$OPT.ll"

# 2. Run your LLVM pass
$LLVM_OPT \
    -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
    --passes="$PLUGIN" \
    -S "$OUTDIR/input_O$OPT.ll" \
    -o "$OUTDIR/instrumented.ll"

# 3. Compile runtime sources to LLVM IR
# $LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm "$FP_RUNTIME_SRC"   -o "$OUTDIR/fp_runtime_O$OPT.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude "$SMEM_RUNTIME_SRC" -o "$OUTDIR/smem_runtime_O$OPT.ll"
# $LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm "$MPFR_RUNTIME_SRC" -o "$OUTDIR/mpfr_runtime_O$OPT.ll"

# 4. Link instrumented IR + runtime IR
$LLVM_LINK \
    "$OUTDIR/instrumented.ll" \
    "$OUTDIR/smem_runtime_O$OPT.ll" \
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
"$OUTDIR/a.out"