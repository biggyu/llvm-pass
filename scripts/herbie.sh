#!/usr/bin/env bash
# Usage: bash scripts/herbie.sh [MAX]
# Runs OPT as the outer loop, then sample and worst for each OPT.
# Builds once per OPT and reuses that build for both modes.
set -u

MAX=${1:-0}
SINGLE_SCRIPT="./scripts/herbie_single.sh"

case "$MAX" in ''|*[!0-9]*) echo "[ERROR] MAX must be a non-negative integer"; exit 1;; esac

if [ ! -f "$SINGLE_SCRIPT" ]; then
    echo "[ERROR] Herbie single script not found: $SINGLE_SCRIPT"
    exit 1
fi

OPTS="0 1 2"
MODES="sample worst"
total=0; completed=0; failed=0

for opt in $OPTS; do
    echo
    echo "################################################################"
    echo "Building once for O$opt"
    echo "################################################################"

    rm -rf ./build
    if ! bash ./scripts/build.sh 0 1 "$opt"; then
        echo "[ERROR] Build failed for O$opt"
        failed=$((failed + 2))
        continue
    fi

    for mode in $MODES; do
        total=$((total + 1))
        echo
        echo "================================================================"
        echo "Herbie run $total: mode=$mode opt=O$opt max=$MAX"
        echo "================================================================"

        if HERBIE_SKIP_BUILD=1 bash "$SINGLE_SCRIPT" "$MAX" "$mode" "$opt"; then
            completed=$((completed + 1))
        else
            failed=$((failed + 1))
            echo "[ERROR] Configuration failed: mode=$mode opt=O$opt"
        fi
    done
done

echo
echo "################################################################"
echo "All Herbie configurations complete"
echo "################################################################"
echo "Completed: $completed"
echo "Failed   : $failed"
echo
echo "Timing CSV files are under:"
echo "  benchmarks/herbie-arith25/produced/"

[ "$failed" -eq 0 ]
