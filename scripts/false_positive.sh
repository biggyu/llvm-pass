#!/usr/bin/env bash
set -u
PROD="./benchmarks/herbie-arith25/produced/sample"
CLEAN_LIST="$PROD/clean_benchmarks.txt"
EFT="../inline/$PROD"
COND="$PROD"

flagged_eft() {   # bits/nan/inf only (EFT-only has no cond signal)
    local f="$1"; [ -f "$f" ] || { echo X; return; }
    v(){ awk -v p="$1" '$0~p{for(i=1;i<=NF;i++)if($i~/^[0-9]+$/)x=$i}END{print x+0}' "$2"; }
    { [ "$(v 'Error above bits' "$f")" -gt 0 ] || [ "$(v 'Total NaN' "$f")" -gt 0 ] || [ "$(v 'Total Inf' "$f")" -gt 0 ]; } && echo 1 || echo 0
}
flagged_cond() {  # includes condition detections
    local f="$1"; [ -f "$f" ] || { echo X; return; }
    v(){ awk -v p="$1" '$0~p{for(i=1;i<=NF;i++)if($i~/^[0-9]+$/)x=$i}END{print x+0}' "$2"; }
    { [ "$(v 'Error above bits' "$f")" -gt 0 ] || [ "$(v 'Total NaN' "$f")" -gt 0 ] || [ "$(v 'Total Inf' "$f")" -gt 0 ] || [ "$(v 'Condition-number detections' "$f")" -gt 0 ]; } && echo 1 || echo 0
}

eft_fp=0; cond_fp=0; total=0; skip=0
while read -r n; do
    [ -n "$n" ] || continue
    e=$(flagged_eft "$EFT/$n/error.log")
    c=$(flagged_cond "$COND/$n/error.log")
    { [ "$e" = X ] || [ "$c" = X ]; } && { skip=$((skip+1)); continue; }
    total=$((total+1))
    [ "$e" -eq 1 ] && eft_fp=$((eft_fp+1))
    [ "$c" -eq 1 ] && cond_fp=$((cond_fp+1))
    { [ "$e" -eq 1 ] || [ "$c" -eq 1 ]; } && printf "  %-50s eft=%s cond=%s\n" "$n" "$e" "$c"
done < "$CLEAN_LIST"

echo
echo "Clean benchmarks tested: $total (skipped $skip)"
echo "EFT-only false positives:      $eft_fp"
echo "EFT+condition false positives: $cond_fp"
[ "$total" -gt 0 ] && awk -v e=$eft_fp -v c=$cond_fp -v n=$total \
    'BEGIN{printf "FP rate:  EFT-only %.1f%%   EFT+cond %.1f%%\n",100*e/n,100*c/n}'