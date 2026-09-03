#!/usr/bin/env bash
set -u
# Usage: build_bench.sh <name> <src_dir> <run_args...>
# sources are auto-discovered (recursive) from src_dir; edit EXCLUDE for files to skip

PASS=${PASS:-ShadowMem}; PLUGIN=${PLUGIN:-shadowmem}; OPT=${OPT:-0}
ROOT="$(realpath .)"
PASS_SO="$ROOT/build/passes/$PASS/$PASS.so"
RUNTIME_LIB="$ROOT/build/runtime/libpass_runtime.a"
PRODUCED_DIR="$ROOT/tests/rodinia/produced"
OMP_STUBS="$ROOT/scripts/rodinia/omp_stubs.c"
OMP_INC="$ROOT/scripts/rodinia"

NAME="$1"; SRC_DIR="$2"; shift 2
RUN_ARGS="$@"

OUTDIR="$ROOT/build/out/$PASS/rodinia-$NAME"
mkdir -p "$OUTDIR" "$PRODUCED_DIR"
echo "=== $NAME ==="

# --- discover all .c files recursively, minus excludes ---
# EXCLUDE: GPU/CUDA files, generators, anything not part of the CPU build
EXCLUDE_PAT='cuda|_cuda|\.cu|gpu|kernel_gpu|hurricane_gen|gen_dataset'
SRCS=$(find "$SRC_DIR" -name '*.c' | grep -Ev "$EXCLUDE_PAT" | sort)

if [ -z "$SRCS" ]; then echo "  [FAIL] no sources found in $SRC_DIR"; return 1 2>/dev/null || exit 1; fi
echo "  sources:"; echo "$SRCS" | sed 's/^/    /'

# include dirs: all dirs containing headers, plus omp stub dir
INC_DIRS="-I $OMP_INC"
for d in $(find "$SRC_DIR" -name '*.h' -exec dirname {} \; | sort -u); do
    INC_DIRS="$INC_DIRS -I $d"
done
INC_DIRS="$INC_DIRS -I $SRC_DIR"

CFLAGS="-O$OPT -g -ffp-contract=off -fno-vectorize -fno-slp-vectorize -fno-math-errno -fno-inline \
        -std=gnu89 -Wno-implicit-function-declaration -Wno-implicit-int \
        -Wno-deprecated-non-prototype -Wno-error=int-conversion \
        $INC_DIRS"

# --- 1. compile each source to IR ---
BENCH_LLS=""
i=0
for s in $SRCS; do
    b="bench_$i"; i=$((i+1))
    "$LLVM_CLANG" $CFLAGS -S -emit-llvm "$s" -o "$OUTDIR/$b.ll" \
        || { echo "  [FAIL] emit $s"; return 1 2>/dev/null || exit 1; }
    BENCH_LLS="$BENCH_LLS $OUTDIR/$b.ll"
done

# --- 2. llvm-link into one module ---
"$LLVM_LINK" $BENCH_LLS -S -o "$OUTDIR/combined.ll" \
    || { echo "  [FAIL] llvm-link"; return 1 2>/dev/null || exit 1; }

# --- 3. instrument combined module ---
"$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes="$PLUGIN" \
    -fp-debug-checks=true -fp-debug-metric=0 \
    -S "$OUTDIR/combined.ll" -o "$OUTDIR/instrumented.ll" \
    || { echo "  [FAIL] pass"; return 1 2>/dev/null || exit 1; }

# --- 4. omp stubs ---
"$LLVM_CLANG" -O$OPT -c "$OMP_STUBS" -o "$OUTDIR/omp_stubs.o" \
    || { echo "  [FAIL] omp_stubs"; return 1 2>/dev/null || exit 1; }

# --- 5. compile instrumented + link ---
"$LLVM_CLANGXX" -O$OPT -c "$OUTDIR/instrumented.ll" -o "$OUTDIR/bench.o" \
    || { echo "  [FAIL] compile IR"; return 1 2>/dev/null || exit 1; }
"$LLVM_CLANGXX" -O$OPT "$OUTDIR/bench.o" "$OUTDIR/omp_stubs.o" "$RUNTIME_LIB" \
    -o "$OUTDIR/$NAME" -lm -lmpfr -lgmp \
    || { echo "  [FAIL] link"; return 1 2>/dev/null || exit 1; }

# --- 6. run ---
ERRLOG_DIR="$PRODUCED_DIR" "$OUTDIR/$NAME" $RUN_ARGS \
    > "$OUTDIR/stdout.txt" 2> "$OUTDIR/stderr.txt"
RC=$?
if [ -f "$PRODUCED_DIR/error.log" ]; then
    mv "$PRODUCED_DIR/error.log" "$PRODUCED_DIR/rodinia-$NAME.log"
    echo "  [OK] (exit $RC)"
else
    echo "  [WARN] no error.log (exit $RC)"
fi