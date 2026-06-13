#!/usr/bin/bash
# sh scripts/instruments.sh TwoSum twosum examples/input_for_twosum.ll 0 1

PASS=$1
PLUGIN=$2
SRC=$3
FP_DEBUG=${4:-0}
OPT=$5

if [ $# -ne 5 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT> <FP_DEBUG> <OPT>"
    exit 1
fi

#TODO: error metric
if [ "$FP_DEBUG" -eq 0 ]; then
    FP_DEBUG_CMAKE=false
else
    FP_DEBUG_CMAKE=true
fi

OUTDIR=build/out/$PASS
mkdir -p "$OUTDIR"

# -ffp-contract=off prevents llvm.fmuladd instructions
# -fno-math-errno: II X: CI
$LLVM_CLANGXX -O"$OPT" -g -S -ffp-contract=off -emit-llvm $SRC -o "$OUTDIR/input_O$OPT.ll"
# $LLVM_CLANGXX -O"$OPT" -g -S -fno-math-errno -emit-llvm  "$SRC" -o "$OUTDIR/input_O$OPT.ll"
# $LLVM_CLANGXX -O"$OPT" -g -S -fno-math-errno -ffp-contract=off -emit-llvm $SRC -o "$OUTDIR/input_O$OPT.ll"

$LLVM_OPT \
    -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
    --passes="$PLUGIN" \
    -fp-debug-checks="$FP_DEBUG_CMAKE" \
    -fp-debug-metric=0 \
    -S "$OUTDIR/input_O$OPT.ll" \
    -o "$OUTDIR/instrumented.ll"

$LLVM_OPT -O"$OPT" \
    -S "$OUTDIR/instrumented.ll" \
    -o "$OUTDIR/optimized_O$OPT.ll"

$LLVM_CLANGXX -O"$OPT" \
    -c "$OUTDIR/optimized_O$OPT.ll" \
    -o "$OUTDIR/optimized_O$OPT.o"

$LLVM_CLANGXX -O"$OPT" \
    "$OUTDIR/optimized_O$OPT.o" \
    "./build/runtime/libpass_runtime.a" \
    -o "$OUTDIR/a.out" \
    -lm -lmpfr -lgmp

for i in 1 2 3;
do
    "$OUTDIR/a.out"
done
# "$OUTDIR/a.out"