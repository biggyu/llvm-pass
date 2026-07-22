#!/usr/bin/env bash
set -u

PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
PROFILE=${3:-0}
FP_DEBUG=${4:-0}
OPT=${5:-0}

# # Guard against unset toolchain vars (the likely "won't run" cause)
# : "${LLVM_CLANG:?set LLVM_CLANG (e.g. export LLVM_CLANG=clang)}"
# : "${LLVM_CLANGXX:?set LLVM_CLANGXX (e.g. export LLVM_CLANGXX=clang++)}"
# : "${LLVM_OPT:?set LLVM_OPT (e.g. export LLVM_OPT=opt)}"

SRC_DIR="./tests/controlled/cases"
PRODUCED_DIR="./tests/controlled/produced"

if [ ! -d "$SRC_DIR" ]; then
    echo "Source directory not found: $SRC_DIR"
    exit 1
fi

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$FP_DEBUG" "$OPT"

mkdir -p "$PRODUCED_DIR"

PASS_SO="./build/passes/$PASS/$PASS.so"
RUNTIME_LIB="./build/runtime/libpass_runtime.a"

if [ ! -f "$PASS_SO" ] || [ ! -f "$RUNTIME_LIB" ]; then
    echo "BUILD FAILED: pass or runtime not produced. Aborting."
    echo "  PASS_SO=$PASS_SO"
    echo "  RUNTIME_LIB=$RUNTIME_LIB"
    exit 1
fi

total=0
ok=0

for BENCH_SRC in "$SRC_DIR"/*.c "$SRC_DIR"/*.cpp; do
    [ -e "$BENCH_SRC" ] || continue

    BENCH_BASENAME=$(basename "$BENCH_SRC")
    BENCH_BASENAME="${BENCH_BASENAME%.*}"
    ext="${BENCH_SRC##*.}"

    total=$((total + 1))
    echo "=== $BENCH_BASENAME ($ext) ==="

    OUTDIR="build/out/${PASS}/${BENCH_BASENAME}"
    mkdir -p "$OUTDIR"
    # PROD_SUBDIR="$PRODUCED_DIR/$BENCH_BASENAME"
    # mkdir -p "$PROD_SUBDIR"

    if [ "$ext" = "cpp" ]; then FRONTEND="$LLVM_CLANGXX"; else FRONTEND="$LLVM_CLANG"; fi

    $FRONTEND -O"$OPT" -g -S -emit-llvm -ffp-contract=off \
        "$BENCH_SRC" -o "$OUTDIR/bench.ll" || { echo "  [FAIL] emit-llvm"; continue; }

    $LLVM_OPT -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
        -fp-debug-checks=true -fp-debug-metric=0 \
        -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.instrumented.ll" || { echo "  [FAIL] opt pass"; continue; }

    $LLVM_CLANGXX -O"$OPT" -c "$OUTDIR/bench.instrumented.ll" \
        -o "$OUTDIR/bench.instrumented.o" || { echo "  [FAIL] compile IR"; continue; }

    $LLVM_CLANGXX -O"$OPT" "$OUTDIR/bench.instrumented.o" "$RUNTIME_LIB" \
        -o "$OUTDIR/a.out" -lm -lmpfr -lgmp || { echo "  [FAIL] link"; continue; }

    # FPCHECK_THRESHOLD=1e3 ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/a.out" > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
    ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/a.out" > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
    RC=$?
    if [ -f "$PRODUCED_DIR/error.log" ]; then
        mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/$BENCH_BASENAME.log"
        echo "  [OK] (exit $RC)"; ok=$((ok + 1))
    else
        echo "  [WARN] no error.log (exit $RC)"
    fi
done

echo
echo "Done: $ok / $total benchmarks produced a log"
