#!/usr/bin/env python3
"""Compare your tool's produced error.logs against Herbie's timeline.json references.
Agreement-based: does your tool detect error where Herbie does, and of a compatible kind?"""

import json, re, os, glob, sys

# ---------- read Herbie reference ----------
def herbie_reference(timeline_path):
    """Return {has_error, top_expr, top_count, error_ops, likely_overflow, likely_cancel}."""
    try:
        tl = json.load(open(timeline_path))
    except Exception:
        return None
    fperrors = None
    for phase in tl:
        if isinstance(phase, dict) and "fperrors" in phase:
            fperrors = phase["fperrors"]; break
    if fperrors is None:
        return None

    error_ops = []
    for e in fperrors:
        expr = e[0]
        c1 = e[1] if len(e) > 1 else 0
        inputsA = e[3] if len(e) > 3 and isinstance(e[3], list) else None
        c3 = e[4] if len(e) > 4 else 0
        inputsB = e[5] if len(e) > 5 and isinstance(e[5], list) else None

        count = max(c1, c3)                 # error in EITHER slot
        if count <= 0:                      # <-- only gate on count, NOT worst inputs
            continue
        worst = inputsB or inputsA          # may be None; that's fine
        error_ops.append({"expr": expr, "count": count, "worst": worst})

    if not error_ops:
        return {"has_error": False, "top_expr": None, "top_count": 0,
                "error_ops": [], "likely_overflow": False, "likely_cancel": False}

    top = max(error_ops, key=lambda o: o["count"])
    expr = top["expr"]
    worst = top["worst"]

    # error-class heuristics — guard against worst being None
    big = bool(worst) and any(abs(float(x)) > 1e150 for x in worst)
    has_sub = "-.f64" in expr or "-.f32" in expr
    has_div = "/.f64" in expr or "/.f32" in expr
    has_mul = "*.f64" in expr or "*.f32" in expr
    likely_overflow = big and (has_div or has_mul)
    likely_cancel = has_sub

    return {
        "has_error": True,
        "top_expr": expr,
        "top_count": top["count"],
        "error_ops": error_ops,
        "likely_overflow": likely_overflow,
        "likely_cancel": likely_cancel,
    }

# ---------- read your produced error.log ----------
def parse_error_log(log_path):
    """Extract counters from your error.log by matching line text."""
    if not os.path.isfile(log_path):
        return None
    txt = open(log_path).read()
    def num(pattern, last=False):
        m = re.findall(pattern + r'.*?(\d+)', txt)
        if not m: return 0
        return int(m[-1] if last else m[0])
    return {
        "above_thres":  num(r"Error above bits", last=True),  # skip threshold, take count
        "nan":          num(r"Total NaN found"),
        "inf":          num(r"Total Inf found"),
        "branch_flips": num(r"Total branch flips found"),
        "cond_detected":num(r"Condition-number detections found"),
        "cancellation": num(r"Total cancellation found"),
        "sensitivity":  num(r"Total sensitivity found"),
        "suppressed":   num(r"Total suppressed found"),
    }

# ---------- compare ----------
def classify(ref, prod):
    """Return (verdict, note)."""
    if ref is None:
        return "NO_REF", "no herbie fperrors"
    if prod is None:
        return "NO_PROD", "no produced error.log"

    herbie_error = ref["has_error"]
    # did your tool fire anything indicating error?
    your_error = (prod["above_thres"] > 0 or prod["cond_detected"] > 0
                  or prod["nan"] > 0 or prod["inf"] > 0)

    if not herbie_error and not your_error:
        return "AGREE_CLEAN", "both clean"
    if herbie_error and your_error:
        # both found error — check kind agreement
        if ref["likely_overflow"] and (prod["inf"] > 0 or prod["nan"] > 0):
            return "AGREE_OVERFLOW", "both: overflow/inf"
        if prod["sensitivity"] > 0 and prod["cancellation"] == 0:
            return "AGREE_SENSITIVITY", "condition: sensitivity"
        if ref["likely_cancel"] and prod["cancellation"] > 0:
            return "AGREE_CANCEL", "both: cancellation"
        return "AGREE_ERROR", "both found error (kind differs/unclear)"
    if herbie_error and not your_error:
        if ref["likely_overflow"]:
            return "MISS_OVERFLOW", "herbie found overflow, you missed (phase-3 gap)"
        return "MISS", "herbie found error, you did not"
    if your_error and not herbie_error:
        return "FALSE_POS", "you flagged, herbie clean"
    return "UNKNOWN", ""

def main():
    report_dir = sys.argv[1] if len(sys.argv) > 1 else "herbie-arith25/report"
    produced_dir = sys.argv[2] if len(sys.argv) > 2 else "produced"

    rows = []
    for tl in sorted(glob.glob(os.path.join(report_dir, "*", "timeline.json"))):
        name = os.path.basename(os.path.dirname(tl))
        ref = herbie_reference(tl)
        prod = parse_error_log(os.path.join(produced_dir, name, "error.log"))
        # only report benchmarks you actually ran (produced exists) OR herbie has error
        if prod is None and (ref is None or not ref["has_error"]):
            continue
        verdict, note = classify(ref, prod)
        rows.append((name, verdict, note, ref, prod))

    # summary tally
    tally = {}
    for _, v, _, _, _ in rows:
        tally[v] = tally.get(v, 0) + 1

    # print table
    print(f"{'benchmark':<28} {'verdict':<16} note")
    print("-" * 80)
    for name, verdict, note, ref, prod in rows:
        print(f"{name:<28} {verdict:<16} {note}")

    print("-" * 80)
    print("Summary:")
    for k in sorted(tally):
        print(f"  {k:<16} {tally[k]}")

    # highlight actionable categories
    misses = [r[0] for r in rows if r[1] == "MISS"]
    fps    = [r[0] for r in rows if r[1] == "FALSE_POS"]
    if misses:
        print(f"\nMISS (herbie error, you silent, NOT overflow) — investigate:")
        for m in misses: print(f"  {m}")
    if fps:
        print(f"\nFALSE_POS (you flagged, herbie clean) — investigate:")
        for f in fps: print(f"  {f}")

if __name__ == "__main__":
    main()
