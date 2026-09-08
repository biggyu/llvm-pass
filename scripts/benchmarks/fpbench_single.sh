#!/usr/bin/env bash
# sh scripts/run_benchmark.sh ShadowMem shadowmem benchmarks/fpbench/foo.c benchmarks/driver/foo_main.c 0 0 1

PASS=$1
PLUGIN=$2
BENCH_SRC=${3:-0}
PROFILE=${4:-0}
FP_DEBUG=${5:-0}
OPT=${6:-0}

if [ $# -ne 6 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <BENCH_SRC> <PROFILE> <FP_DEBUG> <OPT>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$FP_DEBUG" "$OPT"

BENCH_BASENAME=$(basename "${BENCH_SRC%.*}")
BENCH_DIR=$(dirname "$BENCH_SRC")
ROOT_DIR=$(dirname "$BENCH_DIR")
DRIVER_SRC="$ROOT_DIR/driver/${BENCH_BASENAME}_main.c"

if [ ! -f "$BENCH_SRC" ]; then
    echo "Benchmark source not found: $BENCH_SRC"
    exit 1
fi

if [ ! -f "$DRIVER_SRC" ]; then
    echo "Driver source not found: $DRIVER_SRC"
    exit 1
fi

OUTDIR=build/out/${PASS}/$(basename "${BENCH_SRC%.*}")
mkdir -p "$OUTDIR"

# 1. Compile benchmark source to LLVM IR
$LLVM_CLANG -O"$OPT" -g -S -emit-llvm -ffp-contract=off "$BENCH_SRC" -o "$OUTDIR/bench.ll"
# $LLVM_CLANG -O0 -g -S -emit-llvm -fno-math-errno -ffp-contract=off "$BENCH_SRC" -o "$OUTDIR/bench.ll"

# 2. Run your LLVM pass on the benchmark IR
$LLVM_OPT \
  -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
  --passes="$PLUGIN" \
  -fp-debug-checks=true \
  -fp-debug-metric=0 \
  -S "$OUTDIR/bench.ll" \
  -o "$OUTDIR/bench.instrumented.ll"

# 3. Compile instrumented benchmark IR to object
$LLVM_CLANGXX -O"$OPT" \
  -c "$OUTDIR/bench.instrumented.ll" \
  -o "$OUTDIR/bench.instrumented.o"

# 4. Compile driver with main()
$LLVM_CLANGXX -O"$OPT" \
  -c "$DRIVER_SRC" \
  -o "$OUTDIR/driver.o"

# 5. Link benchmark + driver + runtime
$LLVM_CLANGXX -O"$OPT" \
  "$OUTDIR/bench.instrumented.o" \
  "$OUTDIR/driver.o" \
  ./build/runtime/libpass_runtime.a \
  -o "$OUTDIR/a.out" \
  -lm -lmpfr -lgmp

# 6. Run
"$OUTDIR/a.out"