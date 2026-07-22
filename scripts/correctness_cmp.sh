#!/usr/bin/env bash
PRODUCED="./benchmarks/correctness_test/produced"
EXPECTED="./benchmarks/correctness_test/expected"

field() { awk -v ln="$1" 'NR==ln {print $NF}' "$2" 2>/dev/null; }

cmp_field() {
    local p="$1" e="$2"
    if [ -z "$p" ] || [ -z "$e" ]; then echo "-"; return; fi
    # numeric compare only if both are integers
    case "$p$e" in
        *[!0-9]*) echo "$p/$e"; return;;   # non-numeric -> just show
    esac
    if [ "$p" -eq "$e" ]; then echo "ok"; else echo "$p/$e"; fi
}

printf "%-22s %10s %8s %8s %8s\n" "benchmark" "Round" "NaN" "Inf" "BFlip"
printf "%-22s %10s\n" "" "(r vs r+c)"

for prod in "$PRODUCED"/*/error.log; do
    name=$(basename "$(dirname "$prod")")
    exp="$EXPECTED/$name/error.log"
    if [ ! -f "$exp" ]; then
        printf "%-22s  (no expected)\n" "$name"
        continue
    fi

    pR=$(field 1 "$prod")
    eRound=$(field 1 "$exp")
    eCanc=$(field 5 "$exp")
    case "$eCanc" in ''|*[!0-9]*) eCanc=0;; esac 
    eCombined=$((eRound + eCanc))

    pN=$(field 2 "$prod"); eN=$(field 2 "$exp")
    pI=$(field 3 "$prod"); eI=$(field 3 "$exp")
    pB=$(field 4 "$prod"); eB=$(field 4 "$exp")

    printf "%-22s %10s %8s %8s %8s\n" "$name" \
        "$(cmp_field "$pR" "$eCombined")" \
        "$(cmp_field "$pN" "$eN")" \
        "$(cmp_field "$pI" "$eI")" \
        "$(cmp_field "$pB" "$eB")"
done