#!/bin/sh

set -u

RUNNER=${RUNNER:-"./scripts/polybench_single.sh"}
MAX=${1:-0}

if [ ! -f "$RUNNER" ]; then
    echo "[ERROR] PolyBench runner not found:"
    echo "  $RUNNER"
    exit 1
fi

case "$MAX" in
    ''|*[!0-9]*)
        echo "[ERROR] MAX must be a non-negative integer"
        exit 1
        ;;
esac

DATASETS="LARGE"
# DATASETS="MINI SMALL MEDIUM LARGE"
OPTS="0 1 2"

total=0
completed=0
failed=0

for opt in $OPTS; do
    for dataset in $DATASETS; do
        total=$((total + 1))

        echo
        echo "============================================================"
        echo "Run $total: dataset=$dataset opt=O$opt max=$MAX"
        echo "============================================================"

        if sh "$RUNNER" "$dataset" "$opt" "$MAX"; then
            completed=$((completed + 1))
        else
            failed=$((failed + 1))
            echo
            echo "[ERROR] PolyBench run failed:"
            echo "  dataset=$dataset"
            echo "  opt=O$opt"
            echo
            echo "Stopping remaining runs."
            exit 1
        fi
    done
done

echo
echo "============================================================"
echo "All PolyBench configurations complete"
echo "============================================================"
echo "Completed : $completed"
echo "Failed    : $failed"
echo
echo "Results are under:"
echo "  benchmarks/polybench/produced/"

