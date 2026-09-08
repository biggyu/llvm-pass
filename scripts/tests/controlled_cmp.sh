#!/usr/bin/env bash
set -u

PRODUCED="./tests/controlled/produced"
EXPECTED="./tests/controlled/expected"

# Pull a counter's numeric value out of a produced log by matching its line text.
# Uses tail -1 so the "Error above bits 50 found 0" line yields the COUNT (0),
# not the threshold (50) — that line has two numbers, we want the last.
getval() {   # $1=logfile  $2=grep-pattern
    grep "$2" "$1" 2>/dev/null | grep -o '[0-9]\+' | tail -1
}

# Map a shorthand counter name (from the expected file) to the log line text.
pattern_for() {   # $1=counter-name -> prints grep pattern
    case "$1" in
        above_thres|abv) echo "Error above bits" ;;
        nan)             echo "Total NaN found" ;;
        inf)             echo "Total Inf found" ;;
        bflip|branch)    echo "Total branch flips found" ;;
        conv)            echo "Total conversion errors found" ;;
        detected)        echo "Condition-number detections found" ;;
        cancellation|cancel) echo "Total cancellation found" ;;
        sensitivity|sens)    echo "Total sensitivity found" ;;
        untyped)         echo "Total untyped found" ;;
        suppressed|suppr)    echo "Total suppressed found" ;;
        *)               echo "" ;;   # unknown -> no pattern
    esac
}

pass=0
fail=0

printf "%-22s %-8s %s\n" "benchmark" "result" "failures"
printf "%-22s %-8s %s\n" "---------" "------" "--------"

for exp in "$EXPECTED"/*.log; do
    [ -e "$exp" ] || continue
    name=$(basename "$exp" .log)
    prod="$PRODUCED/$name.log"

    if [ ! -f "$prod" ]; then
        printf "%-22s %-8s %s\n" "$name" "MISSING" "(no produced log)"
        fail=$((fail + 1))
        continue
    fi

    row_ok=1
    failures=""

    # Each expected line is  counter:want  where want is 'nonzero' or 'zero'
    while IFS=: read -r counter want; do
        # skip blank lines / comments
        case "$counter" in
            ''|\#*) continue ;;
        esac
        # trim surrounding whitespace from want
        want=$(echo "$want" | tr -d '[:space:]')

        pat=$(pattern_for "$counter")
        if [ -z "$pat" ]; then
            failures="$failures ${counter}?(unknown)"
            row_ok=0
            continue
        fi

        got=$(getval "$prod" "$pat")
        got=${got:-0}

        case "$want" in
            nonzero)
                if [ "$got" -eq 0 ] 2>/dev/null; then
                    failures="$failures ${counter}(want>0,got0)"
                    row_ok=0
                fi
                ;;
            zero)
                if [ "$got" -ne 0 ] 2>/dev/null; then
                    failures="$failures ${counter}(want0,got${got})"
                    row_ok=0
                fi
                ;;
            *)
                # allow an exact numeric expectation too, e.g.  nan:1
                case "$want" in
                    ''|*[!0-9]*) failures="$failures ${counter}(badspec:${want})"; row_ok=0 ;;
                    *)
                        if [ "$got" -ne "$want" ] 2>/dev/null; then
                            failures="$failures ${counter}(want${want},got${got})"
                            row_ok=0
                        fi
                        ;;
                esac
                ;;
        esac
    done < "$exp"

    if [ "$row_ok" -eq 1 ]; then
        printf "%-22s %-8s\n" "$name" "PASS"
        pass=$((pass + 1))
    else
        printf "%-22s %-8s %s\n" "$name" "FAIL" "$failures"
        fail=$((fail + 1))
    fi
done

echo "----"
echo "controlled: $pass passed, $fail failed"
[ "$fail" -eq 0 ]