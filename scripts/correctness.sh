#!/usr/bin/env bash
# Usage: sh scripts/run_correctness.sh <PASS> <PLUGIN> <PROFILE> <FP_DEBUG> <OPT>

set -u

PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
PROFILE=${3:-0}
FP_DEBUG=${4:-0}
OPT=${5:-0}

sh scripts/correctness_test.sh $PASS $PLUGIN $PROFILE $FP_DEBUG $OPT

sh scripts/correctness_cmp.sh