# sh scripts/instruments.sh TwoSum twosum examples/input_for_twosum.ll
PASS=$1
PLUGIN=$2
SRC=$3

if [ $# -ne 3 ]; then
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT>"
    exit 1
fi
# CLANG=${LLVM_CLANG:-clang}
# OPT=${LLVM_OPT:-opt}
# CLANGXX=${LLVM_CLANGXX:-clang++}

OUTDIR=build/out/$PASS
mkdir -p "$OUTDIR"

$LLVM_CLANG -O0 -g -S -emit-llvm $SRC -o $OUTDIR/input.ll
$LLVM_OPT -load-pass-plugin ./build/passes/$PASS/$PASS.so --passes="$PLUGIN" -S $OUTDIR/input.ll -o $OUTDIR/instrumented.ll
$LLVM_CLANGXX -c $OUTDIR/instrumented.ll -o $OUTDIR/instrumented.o
g++ $OUTDIR/instrumented.o ./build/runtime/libfp_runtime.a -o $OUTDIR/a.out -lm
$OUTDIR/a.out