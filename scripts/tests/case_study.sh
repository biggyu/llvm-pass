
#!/usr/bin/env bash
set -u
 
PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
OPT=${3:-0}
 
PROJECT_ROOT=$(pwd -P)
CASES_DIR="$PROJECT_ROOT/tests/cases"
SRC_DIR="$PROJECT_ROOT/tests/cases/src"
PRODUCED_DIR="$CASES_DIR/produced"
 
PASS_SO="$PROJECT_ROOT/build/passes/$PASS/$PASS.so"
RUNTIME_LIB="$PROJECT_ROOT/build/runtime/libpass_runtime.a"
 
# ── 1. Clean and rebuild ────────────────────────────────────────────────────
echo "=== Rebuilding from scratch ==="
rm -rf "$PROJECT_ROOT/build"
cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build" \
    -DCMAKE_BUILD_TYPE=Release \
    || { echo "[FAIL] cmake configure"; exit 1; }
cmake --build "$PROJECT_ROOT/build" -- -j"$(nproc)" \
    || { echo "[FAIL] cmake build"; exit 1; }
 
if [ ! -f "$PASS_SO" ]; then
    echo "[FAIL] Pass .so not found after build: $PASS_SO"; exit 1
fi
if [ ! -f "$RUNTIME_LIB" ]; then
    echo "[FAIL] Runtime lib not found after build: $RUNTIME_LIB"; exit 1
fi
echo "=== Build OK ==="
echo
 
# ── 2. Prepare output directory ─────────────────────────────────────────────
mkdir -p "$PRODUCED_DIR"
 
# ── 3. Discover case files ──────────────────────────────────────────────────
# Each .c file directly under tests/cases/ is one case study.
# Subdirectories are ignored (add recursion here if needed later).
CASES=$(find "$SRC_DIR" -maxdepth 1 -name "*.c" | sort)
if [ -z "$CASES" ]; then
    echo "[FAIL] No .c files found under $CASES_DIR"; exit 1
fi
 
total=0; ok=0; failed=0
 
for SRC in $CASES; do
    total=$((total + 1))
    NAME=$(basename "$SRC" .c)
    echo "=== $NAME (O$OPT) ==="
 
    OUTDIR="$PROJECT_ROOT/build/out/$PASS/cases/$NAME"
    mkdir -p "$OUTDIR"
 
    CFLAGS="-O$OPT -g -ffp-contract=off -fno-vectorize -fno-slp-vectorize"
 
    # ── 3a. Emit LLVM IR ────────────────────────────────────────────────────
    "$LLVM_CLANG" $CFLAGS -S -emit-llvm "$SRC" -o "$OUTDIR/bench.ll" \
        || { echo "  [FAIL] emit-llvm"; failed=$((failed+1)); continue; }
 
    # ── 3b. Instrument ──────────────────────────────────────────────────────
    "$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
        -fp-debug-checks=true -fp-debug-metric=0 \
        -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.inst.ll" \
        || { echo "  [FAIL] opt pass"; failed=$((failed+1)); continue; }
 
    # ── 3c. Compile instrumented IR ─────────────────────────────────────────
    "$LLVM_CLANGXX" -O"$OPT" -c "$OUTDIR/bench.inst.ll" -o "$OUTDIR/bench.o" \
        || { echo "  [FAIL] compile IR"; failed=$((failed+1)); continue; }
 
    # ── 3d. Link ────────────────────────────────────────────────────────────
    "$LLVM_CLANGXX" -O"$OPT" "$OUTDIR/bench.o" "$RUNTIME_LIB" \
        -o "$OUTDIR/a.out" -lm -lmpfr -lgmp \
        || { echo "  [FAIL] link"; failed=$((failed+1)); continue; }
 
    # ── 3e. Build uninstrumented baseline ───────────────────────────────────
    "$LLVM_CLANG" $CFLAGS "$SRC" -o "$OUTDIR/baseline.out" -lm \
        || { echo "  [FAIL] link baseline"; failed=$((failed+1)); continue; }
 
    (
        ulimit -s unlimited 2>/dev/null || true
        "$OUTDIR/baseline.out"
    ) > "$OUTDIR/baseline.stdout.txt" 2> "$OUTDIR/baseline.stderr.txt"
    BASE_RC=$?
 
    if [ "$BASE_RC" -ne 0 ]; then
        echo "  [FAIL] uninstrumented baseline exited $BASE_RC"
        if [ -s "$OUTDIR/baseline.stderr.txt" ]; then
            echo "  Baseline stderr:"
            tail -n 20 "$OUTDIR/baseline.stderr.txt"
        fi
        failed=$((failed+1)); continue
    fi
 
    # ── 3f. Run instrumented binary ─────────────────────────────────────────
    rm -f "$PRODUCED_DIR/${NAME}.log"
    (
        ulimit -s unlimited 2>/dev/null || true
        ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/a.out"
    ) > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
    RC=$?
 
    if [ "$RC" -ne 0 ]; then
        if [ "$RC" -eq 139 ]; then
            echo "  [FAIL] SIGSEGV (exit 139)"
        else
            echo "  [FAIL] instrumented run exited $RC"
        fi
        if [ -s "$OUTDIR/stderr.txt" ]; then
            echo "  Instrumented stderr:"
            tail -n 20 "$OUTDIR/stderr.txt"
        fi
        if [ "${CASES_GDB:-0}" = "1" ]; then
            if command -v gdb >/dev/null 2>&1; then
                echo "  Capturing GDB backtrace..."
                (
                    ulimit -s unlimited 2>/dev/null || true
                    ERRLOG_DIR="$PRODUCED_DIR" gdb -q -batch \
                        -ex run -ex "thread apply all bt" \
                        --args "$OUTDIR/a.out"
                ) > "$OUTDIR/gdb.txt" 2>&1 || true
                tail -n 40 "$OUTDIR/gdb.txt"
            else
                echo "  [WARN] CASES_GDB=1 but gdb not installed"
            fi
        else
            echo "  Re-run with CASES_GDB=1 to capture a GDB backtrace."
        fi
        failed=$((failed+1)); continue
    fi
 
    # ── 3g. Rename log and report ───────────────────────────────────────────
    # The runtime writes to $ERRLOG_DIR/error.log; rename per case
    # so multiple cases don't overwrite each other.
    if [ -f "$PRODUCED_DIR/error.log" ]; then
        mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/${NAME}.log"
        echo "  [OK] log -> produced/${NAME}.log"
        ok=$((ok + 1))
    else
        echo "  [WARN] run succeeded but no error.log written"
        ok=$((ok + 1))   # binary ran cleanly; treat as ok
    fi
done
 
# ── 4. Summary ──────────────────────────────────────────────────────────────
echo
echo "Done: $ok / $total case studies produced a log ($failed failed)"
echo
 

