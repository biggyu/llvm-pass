#!usr/bin/bash
# sh scripts/run.sh ShadowMem shadowmem ./examples/input_for_matmul.cpp 0 0 1

PASS=${1:-0}
PLUGIN=${2:-0}
SRC=${3:-0}
PROFILE=${4:-0}
DEBUG=${5:-0}
OPT_FLAG=${6:-0}

if [ $# -ne 6 ]; then 
    echo "Usage: $0 <PASS> <PLUGIN> <INPUT> <PROFILING> <DEBUG> <OPT_FLAG>"
    exit 1
fi

OUTDIR=build/out/org
mkdir -p "$OUTDIR"

rm -rf ./build
sh ./scripts/build.sh "$PROFILE" "$DEBUG" "$OPT_FLAG"
# echo "instruments_lnk.sh"
# sh ./scripts/instruments_lnk.sh "$PASS" "$PLUGIN" "$SRC" "$PROFILE" "$DEBUG" "$OPT_FLAG"
echo "instruments.sh"
sh ./scripts/instruments.sh "$PASS" "$PLUGIN" "$SRC" "$DEBUG" "$OPT_FLAG"