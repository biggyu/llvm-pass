#!/bin/sh

set -u

DATASET=${1:-SMALL}
OPT=${2:-0}
MAX=${3:-0}

# ----------------------------------------------------------------------
# Paths
# ----------------------------------------------------------------------

POLYBENCH_ROOT="./benchmarks/polybench"
SRC_DIR="$POLYBENCH_ROOT/src"
UTIL_DIR="$POLYBENCH_ROOT/utilities"

OUT_DIR="$POLYBENCH_ROOT/out"
PROD_DIR="$POLYBENCH_ROOT/produced"

PASS_SO="./build/passes/ShadowMem/ShadowMem.so"
RUNTIME="./build/runtime/libpass_runtime.a"

THRESH=${FPCHECK_THRESHOLD:-1e15}

# ----------------------------------------------------------------------
# Timing repetitions
#
# The baseline and instrumented binaries are compiled separately because
# PB_REPS is a compile-time macro in the PolyBench benchmark source.
#
# Override defaults from the environment, for example:
#   PB_BASE_REPS=10000 PB_INST_REPS=100 sh scripts/polybench.sh MINI 0
# ----------------------------------------------------------------------

case "$DATASET" in
    MINI)
        PB_BASE_REPS=${PB_BASE_REPS:-100000}
        PB_INST_REPS=${PB_INST_REPS:-500}
        ;;
    SMALL)
        PB_BASE_REPS=${PB_BASE_REPS:-10000}
        PB_INST_REPS=${PB_INST_REPS:-50}
        ;;
    MEDIUM)
        PB_BASE_REPS=${PB_BASE_REPS:-1000}
        PB_INST_REPS=${PB_INST_REPS:-10}
        ;;
    LARGE|EXTRALARGE)
        PB_BASE_REPS=${PB_BASE_REPS:-1}
        PB_INST_REPS=${PB_INST_REPS:-1}
        ;;
esac

TIMING_CSV="$PROD_DIR/polybench_${DATASET}_O${OPT}.csv"

# ----------------------------------------------------------------------
# LLVM tools
# ----------------------------------------------------------------------

: "${LLVM_CLANG:=clang}"
: "${LLVM_CLANGXX:=clang++}"
: "${LLVM_OPT:=opt}"

# ----------------------------------------------------------------------
# Validate arguments and repetition counts (POSIX sh compatible)
# ----------------------------------------------------------------------

case "$DATASET" in
    MINI|SMALL|MEDIUM|LARGE|EXTRALARGE)
        ;;
    *)
        echo "[ERROR] Invalid dataset: $DATASET"
        echo "Expected: MINI, SMALL, MEDIUM, LARGE, or EXTRALARGE"
        exit 1
        ;;
esac

case "$OPT" in
    0|1|2|3)
        ;;
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

case "$PB_BASE_REPS" in
    ''|*[!0-9]*|0)
        echo "[ERROR] PB_BASE_REPS must be a positive integer"
        exit 1
        ;;
esac

case "$PB_INST_REPS" in
    ''|*[!0-9]*|0)
        echo "[ERROR] PB_INST_REPS must be a positive integer"
        exit 1
        ;;
esac

rm -rf ./build
sh ./scripts/build.sh 0 1 "$OPT"

# ----------------------------------------------------------------------
# Validate paths
# ----------------------------------------------------------------------

if [ ! -d "$SRC_DIR" ]; then
    echo "[ERROR] PolyBench source directory not found:"
    echo "  $SRC_DIR"
    exit 1
fi

if [ ! -f "$UTIL_DIR/polybench.c" ]; then
    echo "[ERROR] polybench.c not found:"
    echo "  $UTIL_DIR/polybench.c"
    exit 1
fi

if [ ! -f "$UTIL_DIR/polybench.h" ]; then
    echo "[ERROR] polybench.h not found:"
    echo "  $UTIL_DIR/polybench.h"
    exit 1
fi

if [ ! -f "$PASS_SO" ]; then
    echo "[ERROR] LLVM pass plugin not found:"
    echo "  $PASS_SO"
    exit 1
fi

if [ ! -f "$RUNTIME" ]; then
    echo "[ERROR] Runtime library not found:"
    echo "  $RUNTIME"
    exit 1
fi

# ----------------------------------------------------------------------
# Output directories and benchmark list
# ----------------------------------------------------------------------

mkdir -p "$OUT_DIR" "$PROD_DIR"

printf '%s\n' \
    "benchmark,dataset,opt,base_reps,inst_reps,baseline_total_s,baseline_avg_s,instrumented_total_s,instrumented_avg_s,overhead_ratio,instr_status" \
    > "$TIMING_CSV"

BENCH_LIST="$OUT_DIR/.polybench_benchmarks.$$"
trap 'rm -f "$BENCH_LIST"' EXIT HUP INT TERM

find "$SRC_DIR" -mindepth 1 -maxdepth 1 -type d -print \
    | sort \
    | while IFS= read -r bench_dir; do
        name=$(basename "$bench_dir")
        bench_c="$bench_dir/$name.c"
        if [ -f "$bench_c" ]; then
            printf '%s\n' "$bench_c"
        fi
    done > "$BENCH_LIST"

bench_count=$(wc -l < "$BENCH_LIST" | awk '{print $1}')

if [ "$bench_count" -eq 0 ]; then
    echo "[ERROR] No PolyBench benchmarks found under:"
    echo "  $SRC_DIR"
    exit 1
fi

# ----------------------------------------------------------------------
# Configuration
# ----------------------------------------------------------------------

echo "PolyBench configuration"
echo "-----------------------"
echo "Source          : $SRC_DIR"
echo "Utilities       : $UTIL_DIR"
echo "Dataset         : $DATASET"
echo "Opt             : O$OPT"
echo "Max             : $MAX"
echo "Threshold       : $THRESH"
echo "Benchmarks      : $bench_count"
echo "Baseline reps   : $PB_BASE_REPS"
echo "Instrument reps : $PB_INST_REPS"
echo

# ----------------------------------------------------------------------
# Counters
# ----------------------------------------------------------------------

count=0
ok=0
crash=0
skip=0

# ----------------------------------------------------------------------
# Run benchmarks
# ----------------------------------------------------------------------

while IFS= read -r bench_c; do
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

    # ------------------------------------------------------------------
    # Baseline compile with PB_BASE_REPS
    # ------------------------------------------------------------------

    if ! "$LLVM_CLANG" \
        -O"$OPT" \
        -I"$UTIL_DIR" \
        -I"$bench_dir" \
        -D"${DATASET}_DATASET" \
        -DPOLYBENCH_TIME \
        -DPOLYBENCH_NO_FLUSH_CACHE \
        -DPB_REPS="$PB_BASE_REPS" \
        -ffp-contract=off \
        "$bench_c" \
        "$UTIL_DIR/polybench.c" \
        -lm \
        -o "$OUTDIR/baseline.out" \
        2>"$OUTDIR/baseline.err"
    then
        echo "  [SKIP] baseline compile failed"
        echo "         log: $OUTDIR/baseline.err"

        if [ -s "$OUTDIR/baseline.err" ]; then
            echo
            sed 's/^/         /' "$OUTDIR/baseline.err"
            echo
        fi

        skip=$((skip + 1))
        continue
    fi

    # ------------------------------------------------------------------
    # Baseline run
    # ------------------------------------------------------------------

    if ! "$OUTDIR/baseline.out" \
        >"$OUTDIR/base.txt" \
        2>"$OUTDIR/base.err"
    then
        echo "  [SKIP] baseline run failed"
        echo "         log: $OUTDIR/base.err"
        skip=$((skip + 1))
        continue
    fi

    base_total_s=$(awk '/^[0-9]+([.][0-9]+)?$/ { print; exit }' "$OUTDIR/base.txt")
    base_total_s=${base_total_s:-NA}
    base_avg_s=NA

    if [ "$base_total_s" != "NA" ]; then
        base_avg_s=$(awk -v total="$base_total_s" -v reps="$PB_BASE_REPS" \
            'BEGIN { if (reps > 0) printf "%.12f", total / reps; else print "NA" }')
    fi

    # ------------------------------------------------------------------
    # Emit benchmark LLVM IR with PB_INST_REPS
    #
    # Only the benchmark source is instrumented. polybench.c is compiled
    # separately and linked without instrumentation.
    # ------------------------------------------------------------------

    if ! "$LLVM_CLANG" \
        -O"$OPT" \
        -I"$UTIL_DIR" \
        -I"$bench_dir" \
        -D"${DATASET}_DATASET" \
        -DPOLYBENCH_TIME \
        -DPOLYBENCH_NO_FLUSH_CACHE \
        -DPB_REPS="$PB_INST_REPS" \
        -ffp-contract=off \
        -S \
        -emit-llvm \
        "$bench_c" \
        -o "$OUTDIR/kernel.ll" \
        2>"$OUTDIR/emit.err"
    then
        echo "  [SKIP] kernel emit-llvm failed"
        echo "         log: $OUTDIR/emit.err"

        if [ -s "$OUTDIR/emit.err" ]; then
            echo
            sed 's/^/         /' "$OUTDIR/emit.err"
            echo
        fi

        skip=$((skip + 1))
        continue
    fi

    # ------------------------------------------------------------------
    # Run ShadowMem LLVM pass
    # ------------------------------------------------------------------

    if ! "$LLVM_OPT" \
        -load-pass-plugin "$PASS_SO" \
        --passes=shadowmem \
        -fp-debug-checks=true \
        -fp-debug-metric=0 \
        -S \
        "$OUTDIR/kernel.ll" \
        -o "$OUTDIR/kernel.instr.ll" \
        2>"$OUTDIR/opt.err"
    then
        echo "  [FAIL] opt pass failed"
        echo "         log: $OUTDIR/opt.err"

        if [ -s "$OUTDIR/opt.err" ]; then
            echo
            sed 's/^/         /' "$OUTDIR/opt.err"
            echo
        fi

        crash=$((crash + 1))
        continue
    fi

    # ------------------------------------------------------------------
    # Compile PolyBench harness without instrumentation
    # ------------------------------------------------------------------

    if ! "$LLVM_CLANG" \
        -O"$OPT" \
        -I"$UTIL_DIR" \
        -I"$bench_dir" \
        -D"${DATASET}_DATASET" \
        -DPOLYBENCH_TIME \
        -DPOLYBENCH_NO_FLUSH_CACHE \
        -ffp-contract=off \
        -c \
        "$UTIL_DIR/polybench.c" \
        -o "$OUTDIR/polybench.o" \
        2>"$OUTDIR/harness.err"
    then
        echo "  [SKIP] PolyBench harness compile failed"
        echo "         log: $OUTDIR/harness.err"

        if [ -s "$OUTDIR/harness.err" ]; then
            echo
            sed 's/^/         /' "$OUTDIR/harness.err"
            echo
        fi

        skip=$((skip + 1))
        continue
    fi

    # ------------------------------------------------------------------
    # Link instrumented benchmark
    # ------------------------------------------------------------------

    if ! "$LLVM_CLANGXX" \
        -O"$OPT" \
        "$OUTDIR/kernel.instr.ll" \
        "$OUTDIR/polybench.o" \
        "$RUNTIME" \
        -lm \
        -lmpfr \
        -lgmp \
        -o "$OUTDIR/a.out" \
        2>"$OUTDIR/link.err"
    then
        echo "  [FAIL] instrumented link failed"
        echo "         log: $OUTDIR/link.err"

        if [ -s "$OUTDIR/link.err" ]; then
            echo
            sed 's/^/         /' "$OUTDIR/link.err"
            echo
        fi

        crash=$((crash + 1))
        continue
    fi

    # ------------------------------------------------------------------
    # Run instrumented benchmark
    # ------------------------------------------------------------------

    instr_status=ok
    instr_total_s=NA
    instr_avg_s=NA

    timeout 1800 env \
        FPCHECK_THRESHOLD="$THRESH" \
        ERRLOG_DIR="$PROD_SUB" \
        "$OUTDIR/a.out" \
        >"$OUTDIR/instr.txt" \
        2>"$OUTDIR/instr.err"
    rc=$?

    if [ "$rc" -eq 0 ]; then
        instr_total_s=$(awk '/^[0-9]+([.][0-9]+)?$/ { print; exit }' "$OUTDIR/instr.txt")
        instr_total_s=${instr_total_s:-NA}

        if [ "$instr_total_s" != "NA" ]; then
            instr_avg_s=$(awk -v total="$instr_total_s" -v reps="$PB_INST_REPS" \
                'BEGIN { if (reps > 0) printf "%.12f", total / reps; else print "NA" }')
        else
            instr_status=no_timing
            echo "  [WARN] no instrumented timing value found"
        fi
    else
        if [ "$rc" -eq 124 ]; then
            instr_status=timeout
            echo "  [TIMEOUT] instrumented run exceeded 1800 seconds"
        else
            instr_status=crash
            echo "  [CRASH] instrumented run failed (rc=$rc)"
            echo "          log: $OUTDIR/instr.err"

            if [ -s "$OUTDIR/instr.err" ]; then
                echo
                tail -20 "$OUTDIR/instr.err" | sed 's/^/          /'
                echo
            fi
        fi

        crash=$((crash + 1))
    fi

    # ------------------------------------------------------------------
    # Calculate overhead from average time per kernel invocation
    # ------------------------------------------------------------------

    ratio=NA

    if [ "$base_avg_s" != "NA" ] && [ "$instr_avg_s" != "NA" ]; then
        ratio=$(awk -v base="$base_avg_s" -v instr="$instr_avg_s" \
            'BEGIN { if (base > 0) printf "%.2f", instr / base; else print "NA" }')
        ok=$((ok + 1))
    fi

    # ------------------------------------------------------------------
    # Write timing CSV and console summary
    # ------------------------------------------------------------------

    printf '%s\n' \
        "$name,$DATASET,O$OPT,$PB_BASE_REPS,$PB_INST_REPS,$base_total_s,$base_avg_s,$instr_total_s,$instr_avg_s,$ratio,$instr_status" \
        >> "$TIMING_CSV"

    echo "  baseline     reps=$PB_BASE_REPS total=${base_total_s}s avg=${base_avg_s}s"
    echo "  instrumented reps=$PB_INST_REPS total=${instr_total_s}s avg=${instr_avg_s}s"
    echo "  overhead=${ratio}x status=$instr_status"

    if [ -f "$PROD_SUB/error.log" ]; then
        echo "  error.log produced"
    else
        echo "  [WARN] no error.log"
    fi

    echo
done < "$BENCH_LIST"

# ----------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------

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
