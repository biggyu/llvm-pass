#!/bin/sh

set -u

DATASET=${1:-SMALL}
OPT=${2:-0}
MAX=${3:-0}

POLYBENCH_ROOT="./benchmarks/polybench"
SRC_DIR="$POLYBENCH_ROOT/src"
UTIL_DIR="$POLYBENCH_ROOT/utilities"
OUT_DIR="$POLYBENCH_ROOT/out"
PROD_DIR="$POLYBENCH_ROOT/produced"
PASS_SO="./build/passes/ShadowMem/ShadowMem.so"
RUNTIME="./build/runtime/libpass_runtime.a"
THRESH=${FPCHECK_THRESHOLD:-1e15}
TIMING_CSV="$PROD_DIR/polybench_${DATASET}_O${OPT}.csv"

: "${LLVM_CLANG:=clang}"
: "${LLVM_CLANGXX:=clang++}"
: "${LLVM_OPT:=opt}"

case "$DATASET" in
    MINI|SMALL|MEDIUM|LARGE|EXTRALARGE) ;;
    *)
        echo "[ERROR] Invalid dataset: $DATASET"
        echo "Expected: MINI, SMALL, MEDIUM, LARGE, or EXTRALARGE"
        exit 1
        ;;
esac

case "$OPT" in
    0|1|2|3) ;;
    *)
        echo "[ERROR] Invalid optimization level: $OPT"
        echo "Expected: 0, 1, 2, or 3"
        exit 1
        ;;
esac

case "$MAX" in
    ''|*[!0-9]*)
        echo "[ERROR] MAX must be a non-negative integer"
        exit 1
        ;;
esac

rm -rf ./build
if ! sh ./scripts/build.sh 0 1 "$OPT"; then
    echo "[ERROR] Build failed"
    exit 1
fi

for required in "$SRC_DIR" "$UTIL_DIR/polybench.c" "$UTIL_DIR/polybench.h" "$PASS_SO" "$RUNTIME"; do
    if [ ! -e "$required" ]; then
        echo "[ERROR] Required path not found:"
        echo "  $required"
        exit 1
    fi
done

mkdir -p "$OUT_DIR" "$PROD_DIR"
printf '%s\n' "benchmark,dataset,opt,baseline_ms,instrumented_ms,overhead_ratio,instr_status" > "$TIMING_CSV"

BENCH_LIST="$OUT_DIR/.polybench_benchmarks.$$"
trap 'rm -f "$BENCH_LIST"' EXIT HUP INT TERM

find "$SRC_DIR" -mindepth 1 -maxdepth 1 -type d -print | sort |
while IFS= read -r bench_dir; do
    name=$(basename "$bench_dir")
    bench_c="$bench_dir/$name.c"
    [ -f "$bench_c" ] && printf '%s\n' "$bench_c"
done > "$BENCH_LIST"

bench_count=$(wc -l < "$BENCH_LIST" | awk '{print $1}')
if [ "$bench_count" -eq 0 ]; then
    echo "[ERROR] No PolyBench benchmarks found under:"
    echo "  $SRC_DIR"
    exit 1
fi

echo "PolyBench configuration"
echo "-----------------------"
echo "Source      : $SRC_DIR"
echo "Utilities   : $UTIL_DIR"
echo "Dataset     : $DATASET"
echo "Opt         : O$OPT"
echo "Max         : $MAX"
echo "Threshold   : $THRESH"
echo "Benchmarks  : $bench_count"
echo "Executions  : 1 baseline, 1 instrumented"
echo

count=0
ok=0
crash=0
skip=0
# ./benchmarks/polybench/adi/adi.c\n./benchmarks/polybench/heat-3d/heat-3d\n./benchmarks/polybench/seidel-2d/seidel-2d
while IFS= read -r "bench_c"; do
# while IFS= read -r bench_c; do
    bench_dir=$(dirname "$bench_c")
    name=$(basename "$bench_dir")
    bench_h="$bench_dir/$name.h"

    if [ "$MAX" -ne 0 ] && [ "$count" -ge "$MAX" ]; then
        break
    fi

    count=$((count + 1))
    echo "=== [$count] $name (dataset=$DATASET opt=O$OPT) ==="

    OUTDIR="$OUT_DIR/$name"
    PROD_SUB="$PROD_DIR/$name"
    mkdir -p "$OUTDIR" "$PROD_SUB"

    if [ ! -f "$bench_h" ]; then
        echo "  [SKIP] benchmark header not found"
        echo "         $bench_h"
        skip=$((skip + 1))
        continue
    fi

    rm -f "$OUTDIR/base.txt" "$OUTDIR/base.err" "$OUTDIR/instr.txt" "$OUTDIR/instr.err" "$PROD_SUB/error.log"

    if ! "$LLVM_CLANG" -O"$OPT" -I"$UTIL_DIR" -I"$bench_dir" -D"${DATASET}_DATASET" -DPOLYBENCH_TIME -DPOLYBENCH_NO_FLUSH_CACHE -ffp-contract=off "$bench_c" "$UTIL_DIR/polybench.c" -lm -o "$OUTDIR/baseline.out" 2>"$OUTDIR/baseline.err"; then
        echo "  [SKIP] baseline compile failed"
        echo "         log: $OUTDIR/baseline.err"
        [ -s "$OUTDIR/baseline.err" ] && sed 's/^/         /' "$OUTDIR/baseline.err"
        skip=$((skip + 1))
        continue
    fi

    if ! "$OUTDIR/baseline.out" >"$OUTDIR/base.txt" 2>"$OUTDIR/base.err"; then
        echo "  [SKIP] baseline run failed"
        echo "         log: $OUTDIR/base.err"
        skip=$((skip + 1))
        continue
    fi

    baseline_s=$(awk '/^[0-9]+([.][0-9]+)?$/ { print; exit }' "$OUTDIR/base.txt")
    baseline_s=${baseline_s:-NA}
    baseline_ms=NA

    if [ "$baseline_s" = "NA" ]; then
        echo "  [WARN] no baseline timing value found"
    else
        baseline_ms=$(awk -v seconds="$baseline_s" \
            'BEGIN { printf "%.6f", seconds * 1000.0 }')
    fi

    if ! "$LLVM_CLANG" -O"$OPT" -I"$UTIL_DIR" -I"$bench_dir" -D"${DATASET}_DATASET" -DPOLYBENCH_TIME -DPOLYBENCH_NO_FLUSH_CACHE -ffp-contract=off -S -emit-llvm "$bench_c" -o "$OUTDIR/kernel.ll" 2>"$OUTDIR/emit.err"; then
        echo "  [SKIP] kernel emit-llvm failed"
        echo "         log: $OUTDIR/emit.err"
        skip=$((skip + 1))
        continue
    fi

    if ! "$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes=shadowmem -fp-debug-checks=true -fp-debug-metric=0 -S "$OUTDIR/kernel.ll" -o "$OUTDIR/kernel.instr.ll" 2>"$OUTDIR/opt.err"; then
        echo "  [FAIL] opt pass failed"
        echo "         log: $OUTDIR/opt.err"
        crash=$((crash + 1))
        printf '%s\n' "$name,$DATASET,O$OPT,$baseline_ms,NA,NA,opt_failed" >> "$TIMING_CSV"
        echo
        continue
    fi

    if ! "$LLVM_CLANG" -O"$OPT" -I"$UTIL_DIR" -I"$bench_dir" -D"${DATASET}_DATASET" -DPOLYBENCH_TIME -DPOLYBENCH_NO_FLUSH_CACHE -ffp-contract=off -c "$UTIL_DIR/polybench.c" -o "$OUTDIR/polybench.o" 2>"$OUTDIR/harness.err"; then
        echo "  [SKIP] PolyBench harness compile failed"
        echo "         log: $OUTDIR/harness.err"
        skip=$((skip + 1))
        printf '%s\n' "$name,$DATASET,O$OPT,$baseline_ms,NA,NA,harness_compile_failed" >> "$TIMING_CSV"
        echo
        continue
    fi

    if ! "$LLVM_CLANGXX" -O"$OPT" "$OUTDIR/kernel.instr.ll" "$OUTDIR/polybench.o" "$RUNTIME" -lm -lmpfr -lgmp -o "$OUTDIR/a.out" 2>"$OUTDIR/link.err"; then
        echo "  [FAIL] instrumented link failed"
        echo "         log: $OUTDIR/link.err"
        crash=$((crash + 1))
        printf '%s\n' "$name,$DATASET,O$OPT,$baseline_ms,NA,NA,link_failed" >> "$TIMING_CSV"
        echo
        continue
    fi

    instr_status=ok
    instrumented_s=NA
    instrumented_ms=NA

    timeout 9600 env FPCHECK_THRESHOLD="$THRESH" ERRLOG_DIR="$PROD_SUB" "$OUTDIR/a.out" >"$OUTDIR/instr.txt" 2>"$OUTDIR/instr.err"
    rc=$?

    if [ "$rc" -eq 0 ]; then
        instrumented_s=$(awk '/^[0-9]+([.][0-9]+)?$/ { print; exit }' "$OUTDIR/instr.txt")
        instrumented_s=${instrumented_s:-NA}
        if [ "$instrumented_s" = "NA" ]; then
            instr_status=no_timing
            echo "  [WARN] no instrumented timing value found"
        else
            instrumented_ms=$(awk -v seconds="$instrumented_s"                 'BEGIN { printf "%.6f", seconds * 1000.0 }')
        fi
    else
        if [ "$rc" -eq 124 ]; then
            instr_status=timeout
            echo "  [TIMEOUT] instrumented run exceeded 9600 seconds"
        else
            instr_status=crash
            echo "  [CRASH] instrumented run failed (rc=$rc)"
            echo "          log: $OUTDIR/instr.err"
            [ -s "$OUTDIR/instr.err" ] && tail -20 "$OUTDIR/instr.err" | sed 's/^/          /'
        fi
        crash=$((crash + 1))
    fi

    ratio=NA
    if [ "$baseline_ms" != "NA" ] && [ "$instrumented_ms" != "NA" ]; then
        ratio=$(awk -v base="$baseline_ms" -v instr="$instrumented_ms" 'BEGIN { if (base > 0) printf "%.2f", instr / base; else print "NA" }')
        ok=$((ok + 1))
    fi

    printf '%s\n' "$name,$DATASET,O$OPT,$baseline_ms,$instrumented_ms,$ratio,$instr_status" >> "$TIMING_CSV"

    echo "  baseline=${baseline_ms}ms"
    echo "  instrumented=${instrumented_ms}ms"
    echo "  overhead=${ratio}x status=$instr_status"

    if [ -f "$PROD_SUB/error.log" ]; then
        echo "  error.log produced"
    else
        echo "  [WARN] no error.log"
    fi
    echo
done < "$BENCH_LIST"

echo "========================================"
echo "PolyBench run complete"
echo "========================================"
echo "Processed     : $count"
echo "OK            : $ok"
echo "Crash/timeout : $crash"
echo "Skipped       : $skip"
echo
echo "Timing CSV:"
echo "  $TIMING_CSV"
