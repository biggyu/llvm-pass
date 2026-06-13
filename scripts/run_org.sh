#!usr/bin/bash
# sh scripts/run_org.sh ./examples/input_for_matmul.c

SRC=$1

if [ $# -ne 1 ]; then 
    echo "Usage: $0 <SRC>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

for OPT in 0 2; do
    echo "=== O$OPT ==="
    $LLVM_CLANGXX -O"$OPT" -ffp-contract=off "$SRC" -o "$OUTDIR/org_O$OPT.out"
    for i in 1 2 3; do
        "$OUTDIR/org_O$OPT.out"
    done
done

# echo "O0 output"
# "$LLVM_CLANGXX" -O0 "$SRC" -o "$OUTDIR/org_O0.out"
# ./"$OUTDIR"/org_O0.out

# echo "O2 output"
# "$LLVM_CLANGXX" -O2 -ffp-contract=off "$SRC" -o "$OUTDIR/org_O2.out"
# ./"$OUTDIR"/org_O2.out