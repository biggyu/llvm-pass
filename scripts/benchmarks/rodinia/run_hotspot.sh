#!/usr/bin/env bash
set -u
PASS=${PASS:-ShadowMem}; PLUGIN=${PLUGIN:-shadowmem}; OPT=${OPT:-0}
ROOT="$(realpath .)"
PASS_SO="$ROOT/build/passes/$PASS/$PASS.so"
RUNTIME_LIB="$ROOT/build/runtime/libpass_runtime.a"
PRODUCED_DIR="$ROOT/tests/rodinia/produced"
OMP_STUBS="$ROOT/scripts/rodinia/omp_stubs.c"

HS_DIR="$ROOT/benchmarks/rodinia/openmp/hotspot"
DATA_DIR="$ROOT/benchmarks/rodinia/data/hotspot"
OUTDIR="$ROOT/build/out/$PASS/rodinia-hotspot"
mkdir -p "$OUTDIR" "$PRODUCED_DIR"

echo "=== hotspot ==="
CFLAGS="-O$OPT -g -ffp-contract=off -fno-vectorize -fno-slp-vectorize -I $ROOT/scripts/rodinia"

# compile omp stubs (native, uninstrumented)
# "$LLVM_CLANGXX" -O$OPT -c "$OMP_STUBS" -o "$OUTDIR/omp_stubs.o" 2>/dev/null \
#     || "$LLVM_CLANG" -O$OPT -c "$OMP_STUBS" -o "$OUTDIR/omp_stubs.o" \
#     || { echo "  [FAIL] omp_stubs"; exit 1; }
"$LLVM_CLANG" -O$OPT -c "$OMP_STUBS" -o "$OUTDIR/omp_stubs.o" \

# hotspot is C++ (.cpp) -> use clang++
"$LLVM_CLANGXX" $CFLAGS -S -emit-llvm "$HS_DIR/hotspot_openmp.cpp" -o "$OUTDIR/bench.ll" \
    || { echo "  [FAIL] emit-llvm"; exit 1; }
"$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
    -fp-debug-checks=true -fp-debug-metric=0 \
    -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.inst.ll" \
    || { echo "  [FAIL] opt pass"; exit 1; }
"$LLVM_CLANGXX" -O$OPT -c "$OUTDIR/bench.inst.ll" -o "$OUTDIR/bench.o" \
    || { echo "  [FAIL] compile IR"; exit 1; }
"$LLVM_CLANGXX" -O$OPT "$OUTDIR/bench.o" "$OUTDIR/omp_stubs.o" "$RUNTIME_LIB" \
    -o "$OUTDIR/hotspot" -lm -lmpfr -lgmp \
    || { echo "  [FAIL] link"; exit 1; }

# run: grid=64x64, 2 iters, 1 thread (small for instrumentation speed)
ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/hotspot" 64 64 2 1 \
    "$DATA_DIR/temp_64" "$DATA_DIR/power_64" "$OUTDIR/output.out" \
    > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
RC=$?
if [ -f "$PRODUCED_DIR/error.log" ]; then
    mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/rodinia-hotspot.log"
    echo "  [OK] (exit $RC)"
else
    echo "  [WARN] no error.log (exit $RC)"
fi