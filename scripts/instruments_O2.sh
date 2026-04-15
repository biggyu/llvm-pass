#!/usr/bin/bash
# sh scripts/instruments_O2.sh ShadowMem shadowmem ./examples/input_for_matmul.c

PASS=$1
PLUGIN=$2
SRC=$3

if [ $# -ne 3 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT>"
    exit 1
fi

OUTDIR=build/out/$PASS
mkdir -p "$OUTDIR"

# 1. Generate IR
$LLVM_CLANG -O2 -g -S -emit-llvm -ffp-contract=off "$SRC" -o "$OUTDIR/input.ll"

# 2. Run your pass
$LLVM_OPT \
  -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
  --passes="$PLUGIN" \
  -S "$OUTDIR/input.ll" \
  -o "$OUTDIR/instrumented.ll"

# # 3. Optimize the instrumented IR
# $LLVM_OPT -S "$OUTDIR/instrumented.ll" -o "$OUTDIR/optimized.ll"

# 4. Compile optimized IR with O2
$LLVM_CLANGXX -c "$OUTDIR/instrumented.ll" -o "$OUTDIR/instrumented.o"

# 5. Link with O2
g++ -O2 "$OUTDIR/instrumented.o" ./build/runtime/libpass_runtime.a \
  -o "$OUTDIR/a.out" -lm -lmpfr -lgmp

# 6. Run
"$OUTDIR/a.out"