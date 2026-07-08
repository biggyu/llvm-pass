#!/usr/bin/env bash
# Usage: sh scripts/run_correctness.sh <PASS> <PLUGIN> <PROFILE> <FP_DEBUG> <OPT>

set -u

PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
PROFILE=${3:-0}
FP_DEBUG=${4:-0}
OPT=${5:-0}

SRC_DIR="./benchmarks/correctness_test/src"
PRODUCED_DIR="./benchmarks/correctness_test/produced"

if [ ! -d "$SRC_DIR" ]; then
    echo "Source directory not found: $SRC_DIR"
    exit 1
fi

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$FP_DEBUG" "$OPT"

mkdir -p "$PRODUCED_DIR"

PASS_SO="./build/passes/$PASS/$PASS.so"
RUNTIME_LIB="./build/runtime/libpass_runtime.a"

total=0
ok=0

if [ ! -f "$PASS_SO" ] || [ ! -f "$RUNTIME_LIB" ]; then
    echo "BUILD FAILED: pass or runtime not produced. Aborting."
    echo "  PASS_SO=$PASS_SO"
    echo "  RUNTIME_LIB=$RUNTIME_LIB"
    exit 1
fi
# Iterate over every .c test in src/
# match both .c and .cpp
for BENCH_SRC in "$SRC_DIR"/*.c "$SRC_DIR"/*.cpp; do
    [ -e "$BENCH_SRC" ] || continue   # skip if glob didn't match (no .cpp files, etc.)

    BENCH_BASENAME=$(basename "$BENCH_SRC")
    BENCH_BASENAME="${BENCH_BASENAME%.*}"   # strip extension
    ext="${BENCH_SRC##*.}"                  # get extension: c or cpp

    total=$((total + 1))
    echo "=== $BENCH_BASENAME ($ext) ==="

    OUTDIR="build/out/${PASS}/${BENCH_BASENAME}"
    mkdir -p "$OUTDIR"
    PROD_SUBDIR="$PRODUCED_DIR/$BENCH_BASENAME"
    mkdir -p "$PROD_SUBDIR"

    # Choose frontend by extension
    if [ "$ext" = "cpp" ]; then
        FRONTEND="$LLVM_CLANGXX"
    else
        FRONTEND="$LLVM_CLANG"
    fi

    # 1. Compile to IR with the right frontend
    $FRONTEND -O"$OPT" -g -S -emit-llvm -ffp-contract=off \
        "$BENCH_SRC" -o "$OUTDIR/bench.ll"
    if [ $? -ne 0 ]; then echo "  [FAIL] emit-llvm"; continue; fi

    # 2. Pass (unchanged)
    $LLVM_OPT -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
        -fp-debug-checks=true -fp-debug-metric=0 \
        -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.instrumented.ll"
    if [ $? -ne 0 ]; then echo "  [FAIL] opt pass"; continue; fi

    # 3. Compile instrumented IR (clang++ works for both C and C++ IR)
    $LLVM_CLANGXX -O"$OPT" -c "$OUTDIR/bench.instrumented.ll" \
        -o "$OUTDIR/bench.instrumented.o"
    if [ $? -ne 0 ]; then echo "  [FAIL] compile IR"; continue; fi

    # 4. Link
    $LLVM_CLANGXX -O"$OPT" "$OUTDIR/bench.instrumented.o" "$RUNTIME_LIB" \
        -o "$OUTDIR/a.out" -lm -lmpfr -lgmp
    if [ $? -ne 0 ]; then echo "  [FAIL] link"; continue; fi

    # 5. Run
    ERRLOG_DIR="$PROD_SUBDIR" "$OUTDIR/a.out" > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
    RC=$?
    if [ -f "$PROD_SUBDIR/error.log" ]; then
        echo "  [OK] (exit $RC)"; ok=$((ok + 1))
    else
        echo "  [WARN] no error.log (exit $RC)"
    fi
done

echo
echo "Done: $ok / $total benchmarks produced an error.log"
