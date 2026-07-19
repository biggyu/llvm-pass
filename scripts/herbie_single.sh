#!/usr/bin/env bash
# Usage: sh run_herbie_validation.sh [MAX] [MODE] [OPT]
#   MAX  = max benchmarks (default 5; 0 = all)
#   MODE = "sample" (timed, loops Herbie's points) or "worst" (single input)
#   OPT  = optimization level for baseline+instrumented (default 0)
set -u

MAX=${1:-5}
MODE=${2:-sample}
OPT=${3:-0}

REPORT_DIR="./benchmarks/herbie-arith25/report"
SRC_DIR="./benchmarks/herbie-arith25/src"
OUT_DIR="./benchmarks/herbie-arith25/expected"
PROD_DIR="./benchmarks/herbie-arith25/produced"
PASS_SO="./build/passes/ShadowMem/ShadowMem.so"
RUNTIME="./build/runtime/libpass_runtime.a"
THRESH="${FPCHECK_THRESHOLD:-1e15}"
TIMING_CSV="$PROD_DIR/timing_${MODE}_O${OPT}.csv"

: "${LLVM_CLANG:=clang}"
: "${LLVM_CLANGXX:=clang++}"
: "${LLVM_OPT:=opt}"

rm -rf ./build
sh ./scripts/build.sh 0 1 "$OPT"

mkdir -p "$SRC_DIR" "$OUT_DIR" "$PROD_DIR"
echo "benchmark,mode,opt,npts,reps,baseline_ms,instrumented_ms,overhead_ratio" > "$TIMING_CSV"

count=0; gen_ok=0; run_ok=0; clean=0; skip=0; fail=0

for tl in "$REPORT_DIR"/*/timeline.json; do
    [ -e "$tl" ] || continue
    if [ "$MAX" -ne 0 ] && [ "$count" -ge "$MAX" ]; then break; fi
    count=$((count + 1))

    name=$(basename "$(dirname "$tl")")
    src="$SRC_DIR/test_$name.c"
    points="$(dirname "$tl")/points.json"

    echo "=== [$count] $name (mode=$MODE opt=O$OPT) ==="

    # 1. generate C
    if [ "$MODE" = "sample" ] && [ -f "$points" ]; then
        python3 ./benchmarks/herbie-arith25/parse_herbie.py "$tl" "$src" \
            --sample "$points" > "$OUT_DIR/$name.gen.log" 2>&1
    else
        python3 ./benchmarks/herbie-arith25/parse_herbie.py "$tl" "$src" \
            > "$OUT_DIR/$name.gen.log" 2>&1
    fi
    rc=$?
    case $rc in
        0)  echo "  [GEN]" ;;
        10) echo "  [CLEAN]"; clean=$((clean+1)); continue ;;
        4)  echo "  [SKIP] unsupported op"; skip=$((skip+1)); continue ;;
        *)  echo "  [FAIL] parse (rc=$rc)"; fail=$((fail+1)); continue ;;
    esac
    gen_ok=$((gen_ok + 1))

    OUTDIR="$OUT_DIR/$name"; mkdir -p "$OUTDIR"
    PROD_SUB="$PROD_DIR/$name"; mkdir -p "$PROD_SUB"

        # 2. baseline binary
    echo "  [BASELINE COMPILE]"

    if ! "$LLVM_CLANG" -O"$OPT" -g -ffp-contract=off \
         "$src" -lm \
         -o "$OUTDIR/baseline.out" \
         2>"$OUTDIR/baseline.err"; then
        echo "  [FAIL] baseline compile"
        fail=$((fail + 1))
        continue
    fi

    echo "  [EMIT LLVM]"

    if ! "$LLVM_CLANG" -O"$OPT" -g -S -emit-llvm \
         -ffp-contract=off \
         "$src" \
         -o "$OUTDIR/bench.ll" \
         2>"$OUTDIR/emit.err"; then
        echo "  [FAIL] emit-llvm"
        fail=$((fail + 1))
        continue
    fi

    echo "  [OPT PASS]"

    if ! timeout 600 "$LLVM_OPT" \
         -load-pass-plugin "$PASS_SO" \
         --passes=shadowmem \
         -fp-debug-checks=true \
         -fp-debug-metric=0 \
         -S "$OUTDIR/bench.ll" \
         -o "$OUTDIR/bench.instr.ll" \
         2>"$OUTDIR/opt.err"; then

        rc=$?

        if [ "$rc" -eq 124 ]; then
            echo "  [TIMEOUT] opt pass exceeded 600 seconds"
        else
            echo "  [FAIL] opt pass (rc=$rc)"
        fi

        fail=$((fail + 1))
        continue
    fi

    echo "  [LINK]"

    if ! "$LLVM_CLANGXX" -O"$OPT" \
         "$OUTDIR/bench.instr.ll" \
         "$RUNTIME" \
         -lm -lmpfr -lgmp \
         -o "$OUTDIR/a.out" \
         2>"$OUTDIR/link.err"; then
        echo "  [FAIL] link"
        fail=$((fail + 1))
        continue
    fi

    # 4. baseline run
    echo "  [BASELINE RUN]"

    if ! timeout 600 "$OUTDIR/baseline.out" \
         >"$OUTDIR/base.txt" \
         2>"$OUTDIR/base.err"; then

        rc=$?

        if [ "$rc" -eq 124 ]; then
            echo "  [TIMEOUT] baseline exceeded 600 seconds"
        else
            echo "  [FAIL] baseline run (rc=$rc)"
        fi

        fail=$((fail + 1))
        continue
    fi

    base_ms=$(grep '^KERNEL_MS' "$OUTDIR/base.txt" | awk '{print $2}')
    npts=$(grep '^NPTS' "$OUTDIR/base.txt" | awk '{print $2}')
    reps=$(grep '^REPS' "$OUTDIR/base.txt" | awk '{print $2}')

    # 5. instrumented run
    echo "  [INSTRUMENTED RUN]"

    if ! timeout 600 env \
         FPCHECK_THRESHOLD="$THRESH" \
         ERRLOG_DIR="$PROD_SUB" \
         "$OUTDIR/a.out" \
         >"$OUTDIR/instr.txt" \
         2>"$OUTDIR/instr.err"; then

        rc=$?

        if [ "$rc" -eq 124 ]; then
            echo "  [TIMEOUT] instrumented run exceeded 600 seconds"
        else
            echo "  [FAIL] instrumented run (rc=$rc)"
        fi

        fail=$((fail + 1))
        continue
    fi

    instr_ms=$(grep '^KERNEL_MS' "$OUTDIR/instr.txt" | awk '{print $2}')

    # guard against missing timing (worst mode has no KERNEL_MS)
    if [ -z "${base_ms:-}" ] || [ -z "${instr_ms:-}" ]; then
        echo "  [WARN] no KERNEL_MS (worst mode or crash) — timing skipped"
    else
        ratio=$(awk "BEGIN{ if ($base_ms>0) printf \"%.2f\", $instr_ms/$base_ms; else print \"NA\" }")
        echo "$name,$MODE,O$OPT,${npts:-0},${reps:-0},$base_ms,$instr_ms,$ratio" >> "$TIMING_CSV"
        echo "  base=${base_ms}ms instr=${instr_ms}ms overhead=${ratio}x (npts=${npts} reps=${reps})"
    fi

    if [ -f "$PROD_SUB/error.log" ]; then
        run_ok=$((run_ok + 1))
    else
        echo "  [WARN] no error.log"
    fi
done

echo
echo "Processed $count | generated $gen_ok | ran $run_ok | clean $clean | skip $skip | fail $fail"
echo "Timing -> $TIMING_CSV"
