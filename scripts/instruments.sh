#!/usr/bin/bash
# sh scripts/instruments.sh TwoSum twosum examples/input_for_twosum.ll 0

PASS=$1
PLUGIN=$2
SRC=$3
OPT=$4

if [ $# -ne 4 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT> <OPT>"
    exit 1
fi

OUTDIR=build/out/$PASS
mkdir -p "$OUTDIR"

# -ffp-contract=off prevents llvm.fmuladd instructions
# -fno-math-errno prevents llvm.sqrt instructions
# $LLVM_CLANG -O0 -g -S -fno-math-errno -emit-llvm $SRC -o $OUTDIR/input.ll
$LLVM_CLANG -O"$OPT" -g -S -emit-llvm -ffp-contract=off $SRC -o "$OUTDIR/input_O$OPT.ll"
$LLVM_OPT \
    -load-pass-plugin ./build/passes/$PASS/$PASS.so \
    --passes="$PLUGIN" \
    -S $OUTDIR/input_O$OPT.ll \
    -o $OUTDIR/instrumented.ll
if [ $OPT -eq "2" ]; then
    $LLVM_OPT -O"$OPT" \
        -S "$OUTDIR/instrumented.ll" \
        -o "$OUTDIR/optimized_O$OPT.ll"
    $LLVM_CLANGXX -O"$OPT" \
        -c "$OUTDIR/optimized_O$OPT.ll" \
        -o "$OUTDIR/optimized_O$OPT.o"
else
    $LLVM_CLANGXX \
        -c $OUTDIR/instrumented.ll \
        -o $OUTDIR/optimized_O$OPT.o
fi
g++ -O$OPT $OUTDIR/optimized_O$OPT.o ./build/runtime/libpass_runtime.a \
    -o $OUTDIR/a.out -lm -lmpfr -lgmp
$OUTDIR/a.out