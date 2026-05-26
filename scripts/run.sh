#!usr/bin/bash
# sh scripts/run.sh 0 ShadowMem shadowmem ./examples/input_for_matmul.cpp 2

PROFILE=${1:-0}
DEBUG=${2:-0}
PASS=${3:-0}
PLUGIN=${4:-0}
SRC=${5:-0}
OPT_FLAG=${6:-0}

if [ $# -ne 6 ]; then 
    echo "Usage: $0 <PROFILING> <DEBUG> <PASS> <PLUGIN> <INPUT> <OPT_FLAG>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$DEBUG_FLAG" "$OPT_FLAG"
echo "instruments_lnk.sh"
sh ./scripts/instruments_lnk.sh "$PASS" "$PLUGIN" "$SRC" "$OPT_FLAG" "$PROFILE"
echo "instruments.sh"
sh ./scripts/instruments.sh "$PASS" "$PLUGIN" "$SRC" "$OPT_FLAG"