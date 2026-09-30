#!/usr/bin/bash
# Usage: sh instruments_lnk.sh <PASS> <PLUGIN> <PROFILE> <DEBUG> <OPT> <src1.c> [src2.c ...]
# single-file:  sh instruments_lnk.sh ShadowMem shadowmem 0 0 1 examples/matmul.c
# multi-file:   sh instruments_lnk.sh ShadowMem shadowmem 0 0 1 kmeans/cluster.c kmeans/kmeans_clustering.c kmeans/kmeans.c kmeans/getopt.c

PASS=$1; PLUGIN=$2; PROFILE=${3:-0}; FP_DEBUG=${4:-0}; OPT=${5:-0}
shift 5                      # remaining args are the source files
SRCS="$@"                    # list of .c/.cpp files

if [ -z "$SRCS" ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <PROFILE> <DEBUG> <OPT> <src1> [src2 ...]"
    exit 1
fi

RUNTIME_DEFS=""
[ "$PROFILE" -eq 1 ] && RUNTIME_DEFS="${RUNTIME_DEFS}-DENABLE_PROFILE=ON "
if [ "$FP_DEBUG" -eq 1 ]; then
    RUNTIME_DEFS="${RUNTIME_DEFS}-DENABLE_FP_DEBUG=ON "
    FP_DEBUG_CMAKE=false
else
    FP_DEBUG_CMAKE=true
fi

OUTDIR="build/out/$PASS"
mkdir -p "$OUTDIR"

SMEM_RUNTIME_SRC="runtime/smem_runtime.cpp"
MPFR_RUNTIME_SRC="runtime/mpfr_runtime.cpp"
FP_DEBUG_SRC="runtime/fp_debug.cpp"

# --- 1. Compile EACH benchmark source to IR (no pass yet) ---
BENCH_LLS=""
for src in $SRCS; do
    base=$(basename "${src%.*}")
    ext="${src##*.}"
    FE="$LLVM_CLANGXX"     # use clang++ for both .c and .cpp (handles both)
    "$FE" -O"$OPT" -g -S -emit-llvm -fno-math-errno -ffp-contract=off \
        -Iinclude "$src" -o "$OUTDIR/bench_$base.ll" \
        || { echo "[FAIL] emit-llvm $src"; exit 1; }
    BENCH_LLS="$BENCH_LLS $OUTDIR/bench_$base.ll"
done

# --- 2. LINK all benchmark IR into ONE module (before instrumenting) ---
$LLVM_LINK $BENCH_LLS -S -o "$OUTDIR/bench_combined.ll" \
    || { echo "[FAIL] llvm-link benchmark"; exit 1; }

# --- 3. Run the pass on the COMBINED module (isDeclaration now correct) ---
$LLVM_OPT -load-pass-plugin "./build/passes/$PASS/$PASS.so" \
    --passes="$PLUGIN" \
    -fp-debug-checks="$FP_DEBUG_CMAKE" \
    -S "$OUTDIR/bench_combined.ll" -o "$OUTDIR/instrumented.ll" \
    || { echo "[FAIL] pass"; exit 1; }

# --- 4. Compile runtime to IR ---
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$SMEM_RUNTIME_SRC" -o "$OUTDIR/smem_runtime.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$MPFR_RUNTIME_SRC" -o "$OUTDIR/mpfr_runtime.ll"
$LLVM_CLANGXX -O"$OPT" -g -S -emit-llvm -Iinclude $RUNTIME_DEFS "$FP_DEBUG_SRC" -o "$OUTDIR/fp_debug.ll"

# --- 5. Link instrumented benchmark + runtime ---
$LLVM_LINK "$OUTDIR/instrumented.ll" \
    "$OUTDIR/smem_runtime.ll" "$OUTDIR/mpfr_runtime.ll" "$OUTDIR/fp_debug.ll" \
    -S -o "$OUTDIR/linked_O$OPT.ll" \
    || { echo "[FAIL] llvm-link runtime"; exit 1; }

# --- 6. Optimize combined ---
$LLVM_OPT -O"$OPT" -S "$OUTDIR/linked_O$OPT.ll" -o "$OUTDIR/optimized_O$OPT.ll"

# --- 7. Compile to object ---
$LLVM_CLANGXX -O"$OPT" -c "$OUTDIR/optimized_O$OPT.ll" -o "$OUTDIR/optimized_O$OPT.o"

# --- 8. Link executable ---
$LLVM_CLANGXX -O"$OPT" "$OUTDIR/optimized_O$OPT.o" -o "$OUTDIR/a.out" -lm -lmpfr -lgmp

echo "Built: $OUTDIR/a.out"