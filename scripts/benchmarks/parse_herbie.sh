HERBIE_DIR="./benchmarks/herbie-arith25/"
SRC_DIR="./benchmarks/herbie-arith25/src"
PARSE_SCRIPT="./benchmarks/herbie-arith25/parse_herbie.py"
 
# Pass through any extra flags (--sample, --include-clean) to parse_herbie.py
EXTRA_FLAGS="$*"
 
mkdir -p "$SRC_DIR"
 
total=0; ok=0; clean=0; error=0; skip=0
 
for tl in "$HERBIE_DIR"/report/*/timeline.json; do
    [ -f "$tl" ] || continue
    name=$(basename "$(dirname "$tl")")
    out_c="$SRC_DIR/$name.c"
    total=$((total + 1))
 
    # If --sample mode, look for a points.json next to timeline.json
    sample_args=""
    if echo "$EXTRA_FLAGS" | grep -q "\-\-sample"; then
        pts="$(dirname "$tl")/points.json"
        if [ -f "$pts" ]; then
            sample_args="--sample $pts"
        else
            echo "  [WARN] $name: --sample requested but no points.json found, skipping sample mode"
        fi
    fi
 
    result=$(python "$PARSE_SCRIPT" "$tl" "$out_c" $sample_args $EXTRA_FLAGS 2>&1)
    exit_code=$?
 
    case $exit_code in
        0)
            echo "  [OK]    $name"
            ok=$((ok + 1))
            ;;
        10)
            # parse_herbie.py exits 10 for clean benchmarks when --include-clean not set
            echo "  [CLEAN] $name"
            clean=$((clean + 1))
            ;;
        2)
            echo "  [SKIP]  $name — no fperrors in timeline"
            skip=$((skip + 1))
            ;;
        *)
            echo "  [FAIL]  $name — exit $exit_code: $result"
            error=$((error + 1))
            ;;
    esac
done
 
echo
echo "Done: $total benchmarks"
echo "  Generated : $ok"
echo "  Clean     : $clean  (no error, skipped unless --include-clean)"
echo "  No fperrors: $skip"
echo "  Failed    : $error"
echo
echo "C files written to: $SRC_DIR/"