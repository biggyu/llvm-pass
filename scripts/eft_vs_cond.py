
#!/usr/bin/env python3
 
import json
import re
import sys
import os
from pathlib import Path
from collections import defaultdict
 
EFT_ONLY_ROOT = Path(os.environ.get(
    "EFT_ROOT",
    os.path.expanduser("~/EFTSan/llvm-pass-inline/benchmarks/herbie-arith25/produced/sample")
))
COND_ROOT     = Path("./benchmarks/herbie-arith25/produced/sample")
HERBIE_DIR    = Path("./benchmarks/herbie-arith25/report")
CLEAN_LIST    = Path("./scripts/benchmarks/herbie/clean_benchmarks.txt")
OUT_DIR       = Path("./scripts/benchmarks/herbie")
OPT           = os.environ.get("OPT", "O0")
 
# EFT threshold values to sweep (in bits)
SWEEP_THRESHOLDS = [50, 40, 30, 20, 10, 5, 0]
 
# ── Log parsing ───────────────────────────────────────────────────────────────
def parse_log(log_path: Path) -> dict | None:
    if not log_path.exists():
        return None
    txt = log_path.read_text(errors="replace")
    def num(pattern):
        m = re.search(pattern + r'.*?(\d+)', txt)
        return int(m.group(1)) if m else 0
    def fnum(pattern):
        m = re.search(pattern + r'.*?([\d.]+)', txt)
        return float(m.group(1)) if m else 0.0
 
    return {
        "above_thres":   num(r"Error above bits \d+ found"),
        "nan":           num(r"Total NaN found"),
        "inf":           num(r"Total Inf found"),
        "branch_flips":  num(r"Total branch flips found"),
        "cond_detected": num(r"Condition-number detections found"),
        "cancellation":  num(r"Total cancellation found"),
        "sensitivity":   num(r"Total sensitivity found"),
        "suppressed":    num(r"Total suppressed found"),
        "max_bits":      fnum(r"bits=([\d.]+)"),
    }
 
def parse_log_with_threshold(log_path: Path, threshold_bits: float) -> bool:
    """Return True if this log would fire at the given bit threshold."""
    counts = parse_log(log_path)
    if counts is None:
        return False
    return counts["max_bits"] >= threshold_bits
 
# ── Herbie reference ──────────────────────────────────────────────────────────
def herbie_has_error(timeline_path: Path) -> bool:
    try:
        with open(timeline_path) as f:
            tl = json.load(f)
    except Exception:
        return False
    for phase in tl:
        if isinstance(phase, dict) and "fperrors" in phase:
            for e in phase["fperrors"]:
                c1 = e[1] if len(e) > 1 else 0
                c3 = e[4] if len(e) > 4 else 0
                if max(c1, c3) > 0:
                    return True
    return False
 
def herbie_category(timeline_path: Path) -> str:
    try:
        with open(timeline_path) as f:
            tl = json.load(f)
    except Exception:
        return "unknown"
    for phase in tl:
        if isinstance(phase, dict) and "fperrors" in phase:
            fperrors = phase["fperrors"]
            error_ops = []
            for e in fperrors:
                c1 = e[1] if len(e) > 1 else 0
                c3 = e[4] if len(e) > 4 else 0
                count = max(c1, c3)
                if count > 0:
                    worst = (e[5] if len(e) > 5 and isinstance(e[5], list) else None) or \
                            (e[3] if len(e) > 3 and isinstance(e[3], list) else None)
                    error_ops.append({"expr": e[0], "count": count, "worst": worst})
            if not error_ops:
                return "clean"
            top = max(error_ops, key=lambda o: o["count"])
            expr = top["expr"]
            worst = top["worst"]
            big = bool(worst) and any(abs(float(x)) > 1e150 for x in worst)
            has_exp  = "exp.f64" in expr or "exp.f32" in expr
            has_sub  = "-.f64" in expr or "-.f32" in expr
            has_div  = "/.f64" in expr or "/.f32" in expr
            has_mul  = "*.f64" in expr or "*.f32" in expr
            has_trig = any(op in expr for op in
                           ("sin.f64","cos.f64","tan.f64",
                            "asin.f64","acos.f64","atan.f64"))
            if big and (has_div or has_mul or has_exp):
                return "overflow"
            if has_sub:
                return "cancellation"
            if has_trig or has_exp:
                return "sensitivity"
            return "error"
    return "unknown"
 
def collect_benchmarks() -> list[str]:
    names = set()
    cond_dir = COND_ROOT / OPT
    if cond_dir.is_dir():
        for p in cond_dir.iterdir():
            if p.is_dir() and (p / "error.log").exists():
                if (HERBIE_DIR / p.name / "timeline.json").exists():
                    names.add(p.name)
    return sorted(names)
 
def load_clean_names() -> set[str]:
    if not CLEAN_LIST.exists():
        print(f"[WARN] clean_benchmarks.txt not found at {CLEAN_LIST}")
        print("       Run analyze_herbie.py first to generate it.")
        return set()
    return set(CLEAN_LIST.read_text().splitlines())
 
def eft_category(counts: dict | None) -> str:
    if counts is None:
        return "NO_LOG"
    if counts["inf"] > 0 or counts["nan"] > 0:
        return "overflow"
    if counts["above_thres"] > 0:
        return "error"
    return "clean"
 
def cond_category(counts: dict | None) -> str:
    if counts is None:
        return "NO_LOG"
    if counts["inf"] > 0 or counts["nan"] > 0:
        if counts["sensitivity"] > 0:
            return "overflow-sensitivity"
        return "overflow"
    if counts["above_thres"] > 0:
        return "error"
    if counts["cancellation"] > 0:
        return "cancellation"
    if counts["sensitivity"] > 0:
        return "sensitivity"
    return "clean"
 
def generate_table3(benchmarks: list[str]) -> dict:
    """
    Reproduce the EFT vs EFT+Condition Number table.
    Categories: overflow, cancellation, sensitivity, error, not_detected
    """
    eft_counts   = defaultdict(int)
    cond_counts  = defaultdict(int)
    not_detected_eft  = []  # herbie says error, eft silent
    not_detected_cond = []  # herbie says error, cond silent
 
    for name in benchmarks:
        h_cat = herbie_category(HERBIE_DIR / name / "timeline.json")
        if h_cat in ("clean", "unknown"):
            continue  # only count benchmarks Herbie flags
 
        eft_log  = EFT_ONLY_ROOT / OPT / name / "error.log"
        cond_log = COND_ROOT / OPT / name / "error.log"
 
        eft_c  = parse_log(eft_log)
        cond_c = parse_log(cond_log)
 
        e_cat = eft_category(eft_c)
        c_cat = cond_category(cond_c)
 
        # EFT detection
        if e_cat in ("overflow", "error"):
            eft_counts[e_cat] += 1
        else:
            eft_counts["not_detected"] += 1
            not_detected_eft.append(name)
 
        # EFT+cond detection
        if c_cat in ("overflow", "overflow-sensitivity", "error",
                     "cancellation", "sensitivity"):
            cond_counts[c_cat] += 1
        else:
            cond_counts["not_detected"] += 1
            not_detected_cond.append(name)
 
    return {
        "eft":  dict(eft_counts),
        "cond": dict(cond_counts),
        "not_detected_eft":  not_detected_eft,
        "not_detected_cond": not_detected_cond,
    }
 
def generate_threshold_sweep(benchmarks: list[str],
                              not_detected_at_50: list[str]) -> list[dict]:
    """
    For each threshold, count how many of the 'not detected at 50 bits'
    benchmarks are recovered, and how many total benchmarks fire.
    Uses the EFT-only logs and varies the threshold.
    """
    # We need per-benchmark max_bits from EFT-only logs
    max_bits_map = {}
    for name in benchmarks:
        log = EFT_ONLY_ROOT / OPT / name / "error.log"
        counts = parse_log(log)
        max_bits_map[name] = counts["max_bits"] if counts else 0.0
 
    target_set = set(not_detected_at_50)
    rows = []
    for thresh in SWEEP_THRESHOLDS:
        recovered = sum(1 for n in target_set
                        if max_bits_map.get(n, 0) >= thresh)
        total_firing = sum(1 for n in benchmarks
                           if max_bits_map.get(n, 0) >= thresh)
        rows.append({
            "threshold": thresh,
            "recovered": recovered,
            "total_target": len(target_set),
            "total_firing": total_firing,
        })
    return rows
 
def generate_sensitivity_analysis(benchmarks: list[str]) -> dict:
    sens_names = set()
    ovf_names  = set()
 
    for name in benchmarks:
        cond_log = COND_ROOT / OPT / name / "error.log"
        counts = parse_log(cond_log)
        if counts is None:
            continue
        if counts["sensitivity"] > 0:
            sens_names.add(name)
        if counts["inf"] > 0 or counts["nan"] > 0:
            ovf_names.add(name)
 
    both    = sens_names & ovf_names
    genuine = sens_names - ovf_names
    return {
        "sensitivity": len(sens_names),
        "overflow":    len(ovf_names),
        "both":        len(both),
        "genuine":     len(genuine),
        "both_names":    sorted(both),
        "genuine_names": sorted(genuine),
    }
 
def generate_fp_analysis(benchmarks: list[str],
                         clean_names: set[str]) -> dict:
    eft_fp = 0; cond_fp = 0; total = 0
 
    for name in benchmarks:
        if name not in clean_names:
            continue
        eft_log  = EFT_ONLY_ROOT / OPT / name / "error.log"
        cond_log = COND_ROOT / OPT / name / "error.log"
 
        eft_c  = parse_log(eft_log)
        cond_c = parse_log(cond_log)
 
        if eft_c is None and cond_c is None:
            continue
        total += 1
 
        if eft_c and (eft_c["above_thres"] > 0 or
                      eft_c["nan"] > 0 or eft_c["inf"] > 0):
            eft_fp += 1
 
        if cond_c and (cond_c["above_thres"] > 0 or
                       cond_c["nan"] > 0 or cond_c["inf"] > 0 or
                       cond_c["cond_detected"] > 0):
            cond_fp += 1
 
    return {
        "total": total,
        "eft_fp": eft_fp,
        "cond_fp": cond_fp,
        "eft_rate":  100 * eft_fp  / total if total else 0,
        "cond_rate": 100 * cond_fp / total if total else 0,
    }
 
def main():
    print(f"EFT-only logs : {EFT_ONLY_ROOT / OPT}")
    print(f"EFT+cond logs : {COND_ROOT / OPT}")
    print(f"Opt level     : {OPT}")
 
    # Check EFT-only directory exists
    eft_dir = EFT_ONLY_ROOT / OPT
    if not eft_dir.is_dir():
        print(f"\n[ERROR] EFT-only log directory not found: {eft_dir}")
        print("Set EFT_ROOT env var or run the EFT-only benchmark first.")
        print("Example: EFT_ROOT=~/EFTSan/llvm-pass-inline/benchmarks/herbie-arith25/produced/sample "
              "python3 compare_eft_vs_cond.py")
        sys.exit(1)
 
    print("\nCollecting benchmarks...")
    benchmarks = collect_benchmarks()
    clean_names = load_clean_names()
    print(f"  {len(benchmarks)} benchmarks with EFT+cond logs")
    print(f"  {len(clean_names)} clean benchmarks")
 
    # Check how many also have EFT-only logs
    eft_available = [n for n in benchmarks
                     if (EFT_ONLY_ROOT / OPT / n / "error.log").exists()]
    print(f"  {len(eft_available)} benchmarks with EFT-only logs")
 
    if not eft_available:
        print("\n[ERROR] No EFT-only logs found. Cannot generate comparison tables.")
        sys.exit(1)
 
    print("\nGenerating Table 3 (EFT vs EFT+Cond)...")
    t3 = generate_table3(benchmarks)
 
    print("Generating Table 4 (Threshold sweep)...")
    sweep = generate_threshold_sweep(
        benchmarks, t3["not_detected_eft"]
    )
 
    print("Generating Section 5.3.3 (Sensitivity analysis)...")
    sens = generate_sensitivity_analysis(benchmarks)
 
    print("Generating false-positive analysis...")
    fp = generate_fp_analysis(benchmarks, clean_names)
 
    # ── Print results ─────────────────────────────────────────────────────────
    out_path = OUT_DIR / f"eft_vs_cond_{OPT}.txt"
    with open(out_path, "w") as f:
        def w(s=""): f.write(s + "\n"); print(s)
 
        w(f"EFT vs EFT+Condition Number Comparison — {OPT}")
        w("=" * 62)
        w()
 
        w("── Table 3: EFT vs EFT+Condition Number ──────────────────")
        w(f"  {'Category':<25} {'EFT':>8} {'EFT+Cond':>10}")
        w("  " + "-" * 45)
 
        eft  = t3["eft"]
        cond = t3["cond"]
        for cat in ["overflow", "cancellation", "sensitivity", "error",
                    "not_detected"]:
            e = eft.get(cat, 0)
            c = cond.get(cat, 0)
            label = "Not Detected" if cat == "not_detected" else cat.title()
            w(f"  {label:<25} {e:>8} {c:>10}")
 
        total_herbie_errors = sum(eft.values())
        w(f"  {'Total (Herbie errors)':<25} {total_herbie_errors:>8}")
        w()
        w(f"  Benchmarks EFT misses that EFT+Cond catches: "
          f"{len(t3['not_detected_eft']) - len(t3['not_detected_cond'])}")
        w()
 
        w("── Table 4: EFT-only Threshold Sweep ─────────────────────")
        w(f"  {'Threshold (bits)':<20} {'Recovered':>12} {'Total Firing':>14}")
        w("  " + "-" * 48)
        for row in sweep:
            w(f"  {row['threshold']:<20} "
              f"{row['recovered']}/{row['total_target']:>8} "
              f"{row['total_firing']:>14}")
        w()
 
        w("── Section 5.3.3: Sensitivity / Overflow ─────────────────")
        w(f"  Sensitivity-flagged : {sens['sensitivity']}")
        w(f"  Overflow-flagged    : {sens['overflow']}")
        w(f"  Intersection (both) : {sens['both']}")
        w(f"  Genuine sensitivity : {sens['genuine']}")
        if sens["genuine_names"]:
            w(f"  Genuine benchmarks  :")
            for n in sens["genuine_names"][:10]:
                w(f"    {n}")
        if sens["both_names"]:
            w(f"  Reclassified (overflow-sens):")
            for n in sens["both_names"][:10]:
                w(f"    {n}")
        w()
 
        w("── False-Positive Analysis (Herbie-clean benchmarks) ──────")
        w(f"  Clean benchmarks tested   : {fp['total']}")
        w(f"  EFT-only FP               : {fp['eft_fp']} ({fp['eft_rate']:.1f}%)")
        w(f"  EFT+cond FP               : {fp['cond_fp']} ({fp['cond_rate']:.1f}%)")
        w()
 
        w(f"Full results written to: {out_path}")
 
    print(f"\nDone. Results in {out_path}")
 
if __name__ == "__main__":
    main()
 







