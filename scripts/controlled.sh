#!/usr/bin/env bash
# Usage: sh scripts/controlled.sh <PASS> <PLUGIN> <PROFILE> <FP_DEBUG> <OPT>
set -u

PASS=${1:-ShadowMem}
PLUGIN=${2:-shadowmem}
PROFILE=${3:-0}
FP_DEBUG=${4:-0}
OPT=${5:-0}

sh scripts/controlled_test.sh "$PASS" "$PLUGIN" "$PROFILE" "$FP_DEBUG" "$OPT" || exit 1
sh scripts/controlled_cmp.sh