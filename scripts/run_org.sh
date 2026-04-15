#!usr/bin/bash
# sh scripts/run_org.sh ./examples/input_for_matmul.c

SRC=$1

if [ $# -ne 1 ]; then 
    echo "Usage: $0 <SRC>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

echo "O0 output"
g++ "$SRC" -o "$OUTDIR/org_O0.out"
./"$OUTDIR"/org_O0.out

echo "O2 output"
g++ -O2 "$SRC" -o "$OUTDIR/org_O2.out"
./"$OUTDIR"/org_O2.out