#!/usr/bin/env bash
# scripts/rodinia/run_myocyte.sh
set -u
PASS=${PASS:-ShadowMem}; PLUGIN=${PLUGIN:-shadowmem}; OPT=${OPT:-1}
ROOT="$(realpath .)"
PASS_SO="$ROOT/build/passes/$PASS/$PASS.so"
RUNTIME_LIB="$ROOT/build/runtime/libpass_runtime.a"
PRODUCED_DIR="$ROOT/tests/rodinia/produced"
OMP_STUBS="$ROOT/scripts/rodinia/omp_stubs.c"
OMP_INC="$ROOT/scripts/rodinia"
MY_DIR="$ROOT/benchmarks/rodinia/openmp/myocyte"
OUTDIR="$ROOT/build/out/$PASS/rodinia-myocyte"
mkdir -p "$OUTDIR" "$PRODUCED_DIR"

echo "=== myocyte (main.c only, includes the rest) ==="
CFLAGS="-O$OPT -g -std=gnu89 -ffp-contract=off -fno-vectorize -fno-slp-vectorize -fno-math-errno -fno-inline \
        -Wno-implicit-function-declaration -Wno-implicit-int -Wno-deprecated-non-prototype \
        -I $OMP_INC -I $MY_DIR"

# compile ONLY main.c (it #includes define.c, cam.c, ecc.c, etc.)
"$LLVM_CLANG" $CFLAGS -S -emit-llvm "$MY_DIR/main.c" -o "$OUTDIR/combined.ll" \
    || { echo "  [FAIL] emit main.c"; exit 1; }

"$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
    -fp-debug-checks=true -fp-debug-metric=0 \
    -S "$OUTDIR/combined.ll" -o "$OUTDIR/instrumented.ll" \
    || { echo "  [FAIL] pass"; exit 1; }

"$LLVM_CLANG" -O$OPT -c "$OMP_STUBS" -o "$OUTDIR/omp_stubs.o"
"$LLVM_CLANGXX" -O$OPT -c "$OUTDIR/instrumented.ll" -o "$OUTDIR/bench.o" \
    || { echo "  [FAIL] compile IR"; exit 1; }
"$LLVM_CLANGXX" -O$OPT "$OUTDIR/bench.o" "$OUTDIR/omp_stubs.o" "$RUNTIME_LIB" \
    -o "$OUTDIR/myocyte" -lm -lmpfr -lgmp \
    || { echo "  [FAIL] link"; exit 1; }

# myocyte reads params.txt / y.txt from its own dir -> run from there
( cd "$MY_DIR" && ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/myocyte" 100 1 0 4 \
    > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt" )
RC=$?
if [ -f "$PRODUCED_DIR/error.log" ]; then
    mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/rodinia-myocyte.log"
    echo "  [OK] (exit $RC)"
else
    echo "  [WARN] no error.log (exit $RC)"
fi