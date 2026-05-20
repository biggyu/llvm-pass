#!usr/bin/bash
# sh scripts/run.sh 0 ShadowMem shadowmem ./examples/input_for_matmul.cpp 2

PROFILE=${1:-0}
PASS=${2:-0}
PLUGIN=${3:-0}
SRC=${4:-0}
OPT_FLAG=${5:-0}

if [ $# -ne 5 ]; then 
    echo "Usage: $0 <PROFILING> <PASS> <PLUGIN> <INPUT> <OPT_FLAG>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$OPT_FLAG"
echo "instruments_lnk.sh"
sh ./scripts/instruments_lnk.sh "$PASS" "$PLUGIN" "$SRC" "$OPT_FLAG" "$PROFILE"
echo "instruments.sh"
sh ./scripts/instruments.sh "$PASS" "$PLUGIN" "$SRC" "$OPT_FLAG"