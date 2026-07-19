#!/usr/bin/env bash
# Threshold sweep: does a lower EFT bit threshold catch the cond-only benchmarks,
# and at what false-positive cost?
#
# Usage: sh scripts/bits_sweep.sh [MAX] [MODE] [OPT]
#   MAX  = max benchmarks (default 0 = all)
#   MODE = "sample" (Herbie's points) or "worst" (single input)
#   OPT  = optimization level (default 0)
set -u

MAX=${1:-0}
MODE=${2:-sample}
OPT=${3:-0}

BITS_LIST="50 40 30 20 10 5 0"

REPORT_DIR="./benchmarks/herbie-arith25/report"
SRC_DIR="./benchmarks/herbie-arith25/src"
OUT_DIR="./benchmarks/herbie-arith25/expected"
PROD_DIR="./benchmarks/herbie-arith25/produced"
PASS_SO="./build/passes/ShadowMem/ShadowMem.so"
RUNTIME="./build/runtime/libpass_runtime.a"
THRESH="${FPCHECK_THRESHOLD:-1e15}"

# list of the 39 cond-only benchmark names, one per line
COND_ONLY_FILE="${COND_ONLY_FILE:-./cond_only.txt}"

: "${LLVM_CLANG:=clang}"
: "${LLVM_CLANGXX:=clang++}"
: "${LLVM_OPT:=opt}"

mkdir -p "$SRC_DIR" "$OUT_DIR" "$PROD_DIR"

if [ ! -f "$COND_ONLY_FILE" ]; then
    echo "[WARN] $COND_ONLY_FILE not found — recall column will be empty"
    : > /tmp/cond_only_empty.txt
    COND_ONLY_FILE=/tmp/cond_only_empty.txt
fi

# ----------------------------------------------------------------------
# Helper: Herbie's max error count for a benchmark (0 = oracle says clean)
# ----------------------------------------------------------------------
herbie_count() {   # $1 = benchmark name
    python3 -c "
import json,sys
try:
    tl=json.load(open('$REPORT_DIR/$1/timeline.json'))
    for p in tl:
        if isinstance(p,dict) and 'fperrors' in p:
            print(max(max(e[1] if len(e)>1 else 0, e[4] if len(e)>4 else 0)
                      for e in p['fperrors']))
            sys.exit(0)
    print(0)
except Exception:
    print(-1)
" 2>/dev/null
}

# ----------------------------------------------------------------------
# Build each benchmark ONCE, then run it at every threshold
# ----------------------------------------------------------------------

count=0; gen_ok=0; clean=0; skip=0; fail=0

for tl in "$REPORT_DIR"/*/timeline.json; do
    [ -e "$tl" ] || continue
    if [ "$MAX" -ne 0 ] && [ "$count" -ge "$MAX" ]; then break; fi
    count=$((count + 1))

    name=$(basename "$(dirname "$tl")")
    src="$SRC_DIR/test_$name.c"
    points="$(dirname "$tl")/points.json"

    echo "=== [$count] $name ==="

    # --- generate C ---
    if [ "$MODE" = "sample" ] && [ -f "$points" ]; then
        python3 ./benchmarks/herbie-arith25/parse_herbie.py "$tl" "$src" \
            --sample "$points" > "$OUT_DIR/$name.gen.log" 2>&1
    else
        python3 ./benchmarks/herbie-arith25/parse_herbie.py "$tl" "$src" \
            > "$OUT_DIR/$name.gen.log" 2>&1
    fi
    rc=$?
    case $rc in
        0)  ;;
        10) echo "  [CLEAN]"; clean=$((clean+1)); continue ;;
        4)  echo "  [SKIP] unsupported op"; skip=$((skip+1)); continue ;;
        *)  echo "  [FAIL] parse (rc=$rc)"; fail=$((fail+1)); continue ;;
    esac
    gen_ok=$((gen_ok + 1))

    OUTDIR="$OUT_DIR/$name"; mkdir -p "$OUTDIR"

    # --- emit + instrument + link (ONCE — threshold is read at runtime) ---
    if ! "$LLVM_CLANG" -O"$OPT" -g -S -emit-llvm -ffp-contract=off \
        "$src" -o "$OUTDIR/bench.ll" 2>"$OUTDIR/emit.err"; then
        echo "  [FAIL] emit-llvm"; fail=$((fail+1)); continue
    fi

    if ! timeout 600 "$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes=shadowmem \
        -fp-debug-checks=true -fp-debug-metric=0 \
        -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.instr.ll" 2>"$OUTDIR/opt.err"; then
        echo "  [FAIL] opt pass"; fail=$((fail+1)); continue
    fi

    if ! "$LLVM_CLANGXX" -O"$OPT" "$OUTDIR/bench.instr.ll" "$RUNTIME" \
        -lm -lmpfr -lgmp -o "$OUTDIR/a.out" 2>"$OUTDIR/link.err"; then
        echo "  [FAIL] link"; fail=$((fail+1)); continue
    fi

    # --- run the SAME binary at each threshold ---
    line="  "
    for T in $BITS_LIST; do
        PROD_SUB="$PROD_DIR/bits_$T/$name"; mkdir -p "$PROD_SUB"

        if ! timeout 600 env \
            FPCHECK_THRESHOLD="$THRESH" \
            FPCHECK_BITS="$T" \
            ERRLOG_DIR="$PROD_SUB" \
            "$OUTDIR/a.out" \
            >"$OUTDIR/instr_$T.txt" 2>"$OUTDIR/instr_$T.err"; then
            line="$line T$T=ERR"
            continue
        fi

        abv=$(grep "Error above bits" "$PROD_SUB/error.log" 2>/dev/null \
              | grep -o '[0-9]*' | tail -1)
        line="$line T$T=${abv:-0}"
    done
    echo "$line"
done

echo
echo "Built $gen_ok | clean $clean | skip $skip | fail $fail"

# ----------------------------------------------------------------------
# Analysis: recall on the cond-only set vs false positives, per threshold
# ----------------------------------------------------------------------

echo
echo "======================================================================"
echo "Threshold sweep: EFT bits metric vs the condition-number-only set"
echo "======================================================================"
printf "%-8s %-18s %-18s %s\n" "bits" "caught/39" "false_pos" "total_firing"
printf "%-8s %-18s %-18s %s\n" "----" "---------" "---------" "------------"

TOTAL_COND=$(grep -c . "$COND_ONLY_FILE" 2>/dev/null || echo 0)

# cache herbie counts so we don't re-parse per threshold
HERBIE_CACHE=/tmp/herbie_counts.txt
: > "$HERBIE_CACHE"
for d in "$PROD_DIR/bits_50"/*/; do
    [ -d "$d" ] || continue
    n=$(basename "$d")
    echo "$n $(herbie_count "$n")" >> "$HERBIE_CACHE"
done

for T in $BITS_LIST; do
    caught=0; fp=0; firing=0
    for d in "$PROD_DIR/bits_$T"/*/; do
        [ -d "$d" ] || continue
        n=$(basename "$d")
        [ -f "$d/error.log" ] || continue

        abv=$(grep "Error above bits" "$d/error.log" 2>/dev/null \
              | grep -o '[0-9]*' | tail -1)
        [ "${abv:-0}" -gt 0 ] || continue
        firing=$((firing + 1))

        # is this one of the cond-only benchmarks EFT previously missed?
        if grep -qx "$n" "$COND_ONLY_FILE" 2>/dev/null; then
            caught=$((caught + 1))
        fi

        # false positive: EFT fires but Herbie's oracle reports clean
        hb=$(awk -v k="$n" '$1==k {print $2}' "$HERBIE_CACHE")
        if [ "${hb:-1}" = "0" ]; then
            fp=$((fp + 1))
        fi
    done
    printf "%-8s %-18s %-18s %s\n" "$T" "$caught/$TOTAL_COND" "$fp" "$firing"
done

echo
echo "Interpretation:"
echo "  If EFT needs a low threshold to reach caught=$TOTAL_COND AND false_pos climbs,"
echo "  condition numbers give equivalent recall at better precision."
echo "  If EFT reaches full recall with false_pos=0 at a modest threshold,"
echo "  condition numbers add classification but not detection."