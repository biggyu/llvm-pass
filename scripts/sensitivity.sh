#!/usr/bin/env bash
# Compare sensitivity vs overflow overlap across sample and worst modes.
# Assumes error logs live under:
#   ./benchmarks/herbie-arith25/produced/sample/<name>/error.log
#   ./benchmarks/herbie-arith25/produced/worst/<name>/error.log
set -u

PROD_DIR="./benchmarks/herbie-arith25/produced"
MODES="sample"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# extract an integer counter value from an error.log line, default 0
counter() {   # $1 = file, $2 = label text
    awk -v pat="$2" '
        $0 ~ pat {
            for (i=1;i<=NF;i++) if ($i ~ /^[0-9]+$/) v=$i
        }
        END { print (v=="" ? 0 : v) }
    ' "$1"
}

for mode in $MODES; do
    MODE_DIR="$PROD_DIR/$mode"
    if [ ! -d "$MODE_DIR" ]; then
        echo "[WARN] $MODE_DIR does not exist — did the $mode run write there?"
        continue
    fi

    sens="$TMP/sens_$mode.txt"
    ovf="$TMP/ovf_$mode.txt"
    : > "$sens"; : > "$ovf"

    for d in "$MODE_DIR"/*/; do
        [ -d "$d" ] || continue
        name=$(basename "$d")
        log="$d/error.log"
        [ -f "$log" ] || continue

        s=$(counter "$log" "Total sensitivity found")
        n=$(counter "$log" "Total NaN found")
        i=$(counter "$log" "Total Inf found")

        [ "$s" -gt 0 ] && printf '%s\n' "$name" >> "$sens"
        { [ "$n" -gt 0 ] || [ "$i" -gt 0 ]; } && printf '%s\n' "$name" >> "$ovf"
    done

    sort -o "$sens" "$sens"
    sort -o "$ovf" "$ovf"

    both="$TMP/both_$mode.txt"
    genuine="$TMP/genuine_$mode.txt"
    comm -12 "$sens" "$ovf" > "$both"
    comm -23 "$sens" "$ovf" > "$genuine"

    sc=$(wc -l < "$sens"); oc=$(wc -l < "$ovf")
    bc=$(wc -l < "$both"); gc=$(wc -l < "$genuine")

    # persist results next to the logs
    out="$PROD_DIR/sensitivity_${mode}_result.log"
    {
        echo "Herbie sensitivity vs overflow — mode=$mode"
        echo "================================================"
        echo "Sensitivity-flagged : $sc"
        echo "Overflow            : $oc"
        echo "Intersection        : $bc"
        echo "Genuine sensitivity : $gc"
        echo
        echo "Genuine (sensitivity, not overflow):"
        cat "$genuine"
        echo
        echo "Intersection (sensitivity AND overflow):"
        cat "$both"
    } > "$out"

    echo "=== $mode ==="
    echo "  sensitivity=$sc overflow=$oc intersection=$bc genuine=$gc"
    echo "  -> $out"
    cp "$sens" "$PROD_DIR/sensitivity_${mode}_benchmarks.txt"
    cp "$ovf"  "$PROD_DIR/overflow_${mode}_benchmarks.txt"
    cp "$both" "$PROD_DIR/sensitivity_overflow_${mode}_benchmarks.txt"
    cp "$genuine" "$PROD_DIR/genuine_sensitivity_${mode}_benchmarks.txt"
done

# cross-mode comparison
if [ -f "$TMP/genuine_sample.txt" ] && [ -f "$TMP/genuine_worst.txt" ]; then
    echo
    echo "=== stable genuine-sensitivity core (both modes) ==="
    comm -12 "$TMP/genuine_sample.txt" "$TMP/genuine_worst.txt"
fi