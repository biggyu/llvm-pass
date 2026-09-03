#!/usr/bin/env bash
set -u

PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
CLASS=${3:-S}          # NPB class: S is smallest (use S for instrumentation)
OPT=${4:-1}            # O1 for PHIs

PROJECT_ROOT=$(pwd -P)
NPB_DIR="$PROJECT_ROOT/benchmarks/npb-c"
COMMON_DIR="$NPB_DIR/common"
SYS_DIR="$NPB_DIR/sys"
PRODUCED_DIR="$PROJECT_ROOT/tests/npb/produced"

PASS_SO="$PROJECT_ROOT/build/passes/$PASS/$PASS.so"
RUNTIME_LIB="$PROJECT_ROOT/build/runtime/libpass_runtime.a"

# --- build the pass + runtime (reuse your build) ---
# (assumes already built; if not, run your build.sh here)
if [ ! -f "$PASS_SO" ] || [ ! -f "$RUNTIME_LIB" ]; then
    echo "BUILD the pass/runtime first (build.sh)"; exit 1
fi

# --- compile setparams natively (once, or when its source changes) ---
if [ ! -x "$SYS_DIR/setparams" ] || [ "$SYS_DIR/setparams.c" -nt "$SYS_DIR/setparams" ]; then
    echo "Building setparams (native)..."
    clang "$SYS_DIR/setparams.c" -o "$SYS_DIR/setparams" \
        || { echo "setparams build failed"; exit 1; }
fi

mkdir -p "$PRODUCED_DIR"

# Benchmarks to run (single-file serial NPB). Override from the environment,
# for example: BENCHMARKS="BT CG EP IS" sh scripts/npb.sh
BENCHMARKS=${BENCHMARKS:-"BT CG EP FT IS LU MG SP"}

total=0; ok=0
for BM in $BENCHMARKS; do
    total=$((total + 1))
    bm_lower=$(echo "$BM" | tr 'A-Z' 'a-z')
    BM_DIR="$NPB_DIR/$BM"
    PARAMS_HEADER="$BM_DIR/npbparams.h"
    echo "=== $BM (class $CLASS) ==="

    # setparams writes npbparams.h to its current directory. Recreate the
    # benchmark-local generated header so a stale/incomplete copy cannot win
    # the compiler's quoted-header search over another include directory.
    rm -f "$PARAMS_HEADER"
    ( cd "$BM_DIR" && "$SYS_DIR/setparams" "$bm_lower" "$CLASS" ) \
        || { echo "  [FAIL] setparams"; continue; }

    # Verify the metadata used by c_print_results before starting the build.
    # IS uses descriptive C build macros instead of the CS1-CS6 macros used
    # by the other serial NPB benchmarks.
    case "$BM" in
        IS)
            required_metadata="NPBVERSION COMPILETIME CC CFLAGS CLINK CLINKFLAGS C_LIB C_INC"
            ;;
        *)
            required_metadata="NPBVERSION COMPILETIME CS1 CS2 CS3 CS4 CS5 CS6"
            ;;
    esac

    missing_metadata=""
    for macro in $required_metadata; do
        if ! grep -Eq "^#[[:space:]]*define[[:space:]]+$macro([[:space:]]|$)" "$PARAMS_HEADER"; then
            missing_metadata="$missing_metadata $macro"
        fi
    done
    if [ -n "$missing_metadata" ]; then
        echo "  [FAIL] $PARAMS_HEADER is missing:$missing_metadata"
        echo "         Check that sys/setparams.c matches this NPB source tree."
        continue
    fi

    OUTDIR="$PROJECT_ROOT/build/out/$PASS/npb-$BM"
    mkdir -p "$OUTDIR"

    INCLUDES="-I $BM_DIR -I $COMMON_DIR"
    CFLAGS="-O$OPT -g -ffp-contract=off -fno-openmp -fno-vectorize -fno-slp-vectorize $INCLUDES"

    # 2. compile + instrument the benchmark source
    "$LLVM_CLANG" $CFLAGS -S -emit-llvm "$BM_DIR/$bm_lower.c" -o "$OUTDIR/bench.ll" \
        || { echo "  [FAIL] emit-llvm"; continue; }
    "$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
        -fp-debug-checks=true -fp-debug-metric=0 \
        -S "$OUTDIR/bench.ll" -o "$OUTDIR/bench.inst.ll" \
        || { echo "  [FAIL] opt pass"; continue; }
    "$LLVM_CLANGXX" -O"$OPT" -c "$OUTDIR/bench.inst.ll" -o "$OUTDIR/bench.o" \
        || { echo "  [FAIL] compile IR"; continue; }

    # 3. compile common/ support files (uninstrumented; non-FP support code)
    COMMON_OBJS=""
    for cf in c_print_results c_timers c_randdp wtime; do
        if [ -f "$COMMON_DIR/$cf.c" ]; then
            "$LLVM_CLANG" -O"$OPT" -g $INCLUDES -c "$COMMON_DIR/$cf.c" -o "$OUTDIR/$cf.o" \
                || { echo "  [FAIL] common $cf"; continue 2; }
            COMMON_OBJS="$COMMON_OBJS $OUTDIR/$cf.o"
        fi
    done

    # 4. link benchmark + common + runtime
    "$LLVM_CLANGXX" -O"$OPT" "$OUTDIR/bench.o" $COMMON_OBJS "$RUNTIME_LIB" \
        -o "$OUTDIR/a.out" -lm -lmpfr -lgmp \
        || { echo "  [FAIL] link"; continue; }

    # 5. Build and run an uninstrumented baseline first. This distinguishes
    # an NPB/build problem from a pass/runtime problem.
    "$LLVM_CLANGXX" -O"$OPT" "$OUTDIR/bench.ll" $COMMON_OBJS \
        -o "$OUTDIR/baseline.out" -lm \
        || { echo "  [FAIL] link baseline"; continue; }

    (
        cd "$BM_DIR" || exit 1
        ulimit -s unlimited 2>/dev/null || true
        "$OUTDIR/baseline.out"
    ) > "$OUTDIR/baseline.stdout.txt" 2> "$OUTDIR/baseline.stderr.txt"
    BASE_RC=$?
    if [ "$BASE_RC" -ne 0 ]; then
        echo "  [FAIL] uninstrumented baseline exited $BASE_RC"
        if [ -s "$OUTDIR/baseline.stderr.txt" ]; then
            echo "  Last baseline stderr lines:"
            tail -n 40 "$OUTDIR/baseline.stderr.txt"
        fi
        continue
    fi

    # 6. Run the instrumented binary from the benchmark directory so any NPB
    # input files are resolved in the same way as the standard NPB build.
    rm -f "$PRODUCED_DIR/error.log"
    (
        cd "$BM_DIR" || exit 1
        ulimit -s unlimited 2>/dev/null || true
        ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/a.out"
    ) > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
    RC=$?

    if [ "$RC" -ne 0 ]; then
        if [ "$RC" -eq 139 ]; then
            echo "  [FAIL] instrumented run received SIGSEGV (exit 139)"
        else
            echo "  [FAIL] instrumented run exited $RC"
        fi
        if [ -s "$OUTDIR/stderr.txt" ]; then
            echo "  Last instrumented stderr lines:"
            tail -n 40 "$OUTDIR/stderr.txt"
        fi

        if [ "${NPB_GDB:-0}" = "1" ]; then
            if command -v gdb >/dev/null 2>&1; then
                echo "  Capturing debugger backtrace..."
                (
                    cd "$BM_DIR" || exit 1
                    ulimit -s unlimited 2>/dev/null || true
                    ERRLOG_DIR="$PRODUCED_DIR" gdb -q -batch \
                        -ex run -ex "thread apply all bt" --args "$OUTDIR/a.out"
                ) > "$OUTDIR/gdb.txt" 2>&1 || true
                tail -n 80 "$OUTDIR/gdb.txt"
            else
                echo "  [WARN] NPB_GDB=1 requested, but gdb is not installed"
            fi
        else
            echo "  Re-run with NPB_GDB=1 to capture a debugger backtrace."
        fi
        continue
    fi

    if [ -f "$PRODUCED_DIR/error.log" ]; then
        mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/npb-$BM.log"
        echo "  [OK] (exit $RC)"; ok=$((ok + 1))
    else
        echo "  [WARN] no error.log (exit $RC)"
    fi
done

echo
echo "Done: $ok / $total NPB benchmarks produced a log"
