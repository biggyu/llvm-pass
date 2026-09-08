#!/usr/bin/env bash
set -eu
# Usage: bash asan_bench.sh <src_dir> [run_args...]
# e.g.:  bash asan_bench.sh benchmarks/rodinia/openmp/kmeans/kmeans_serial -i benchmarks/rodinia/data/kmeans/100

ROOT="$(realpath .)"
PASS_SO="$ROOT/build/passes/ShadowMem/ShadowMem.so"
OMP_STUBS="$ROOT/scripts/rodinia/omp_stubs.c"
OMP_INC="$ROOT/scripts/rodinia"
OUT="$ROOT/build/out/ShadowMem/asan"
mkdir -p "$OUT"

SRC_DIR="$1"; shift
RUN_ARGS="$@"

ASAN="-fsanitize=address -fno-omit-frame-pointer"
# old-C relaxation + your recipe (minus asan on the IR-emit step)
COMMON="-O0 -g -ffp-contract=off -fno-vectorize -fno-slp-vectorize -fno-math-errno -fno-inline \
        -Wno-implicit-function-declaration -Wno-implicit-int -Wno-deprecated-non-prototype"

# include dirs
INC="-I $OMP_INC"
for d in $(find "$SRC_DIR" -name '*.h' -exec dirname {} \; | sort -u); do INC="$INC -I $d"; done
INC="$INC -I $SRC_DIR"

EXCLUDE_PAT='cuda|_cuda|\.cu|gpu|hurricane_gen|gen_dataset'
SRCS=$(find "$SRC_DIR" \( -name '*.c' -o -name '*.cpp' \) | grep -Ev "$EXCLUDE_PAT" | sort)

echo "=== 1. compile each source to IR (no asan, no pass) ==="
BENCH_LLS=""
i=0
for s in $SRCS; do
    b="b$i"; i=$((i+1))
    case "$s" in
        *.cpp) FE="$LLVM_CLANGXX"; STD="" ;;
        *)     FE="$LLVM_CLANG";   STD="-std=gnu89" ;;
    esac
    "$FE" $STD $COMMON $INC -S -emit-llvm "$s" -o "$OUT/$b.ll"
    BENCH_LLS="$BENCH_LLS $OUT/$b.ll"
done

echo "=== 2. llvm-link into one module ==="
"$LLVM_LINK" $BENCH_LLS -S -o "$OUT/combined.ll"

echo "=== 3. instrument combined module ==="
"$LLVM_OPT" -load-pass-plugin "$PASS_SO" --passes=shadowmem \
    -fp-debug-checks=true -fp-debug-metric=0 \
    -S "$OUT/combined.ll" -o "$OUT/instrumented.ll"

echo "=== 4. compile instrumented IR to object WITH asan ==="
"$LLVM_CLANGXX" $ASAN -O0 -c "$OUT/instrumented.ll" -o "$OUT/bench.o"

echo "=== 5. runtime sources WITH asan ==="
RT_INCS="-I $ROOT/include -I $ROOT/runtime"
RT_OBJS=""
for f in $ROOT/runtime/*.cpp; do
    b=$(basename "${f%.cpp}")
    "$LLVM_CLANGXX" $ASAN -O0 $RT_INCS -c "$f" -o "$OUT/rt_$b.o"
    RT_OBJS="$RT_OBJS $OUT/rt_$b.o"
done

echo "=== 6. omp stubs WITH asan ==="
"$LLVM_CLANG" $ASAN -O0 -c "$OMP_STUBS" -o "$OUT/omp_stubs.o"

echo "=== 7. link everything WITH asan ==="
"$LLVM_CLANGXX" $ASAN -O0 "$OUT/bench.o" $RT_OBJS "$OUT/omp_stubs.o" \
    -o "$OUT/bench_asan" -lm -lmpfr -lgmp

echo "=== 8. run under asan ==="
ASAN_OPTIONS=abort_on_error=1:halt_on_error=1:detect_leaks=0 \
    "$OUT/bench_asan" $RUN_ARGS