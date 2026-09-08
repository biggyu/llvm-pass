import json
import os
import re
import sys
from pathlib import Path
from collections import defaultdict

# ── Paths ──────────────────────────────────────────────────────────────────
SCRIPT_DIR   = Path("./scripts/benchmarks/")
HERBIE_DIR   = Path("./benchmarks/herbie-arith25/")
PRODUCED_DIR = Path("./benchmarks/herbie-arith25/produced/sample")
OPT_LEVELS   = ["O0", "O1", "O2"]

SCRIPT_DIR.mkdir(parents=True, exist_ok=True)

def herbie_category(timeline_path: Path) -> str:
    try:
        with open(timeline_path) as f:
            timeline = json.load(f)
    except (json.JSONDecodeError, FileNotFoundError):
        return "unknown"

    explain = next((e for e in timeline if e.get("type") == "explain"), None)
    if explain is None:
        return "unknown"

    confusion = explain.get("confusion", [[0, 0, 0, 0]])[0]

    fperrors = explain.get("fperrors", [])
    total_errors = sum(fe[1] for fe in fperrors if isinstance(fe[1], (int, float)))

    if total_errors == 0:
        return "clean"

    expl_stats = explain.get("expl-stats", [])

    expr_names = [fe[0] for fe in fperrors if isinstance(fe[0], str)]
    overflow_ops = {"exp", "pow", "expt"}
    cancellation_ops = {"-.f64", "-.f32", "+.f64", "+.f32", "log.f64", "log.f32"}
    sensitivity_ops  = {"sin.f64", "cos.f64", "tan.f64", "exp.f64", "exp.f32",
                        "pow.f64", "asin.f64", "acos.f64", "atan.f64"}

    expr_text = " ".join(expr_names)

    has_overflow    = any(op in expr_text for op in overflow_ops)
    has_cancellation = any(op in expr_text for op in cancellation_ops)
    has_sensitivity  = any(op in expr_text for op in sensitivity_ops)

    if has_overflow:
        return "overflow"
    if has_cancellation:
        return "cancellation"
    if has_sensitivity:
        return "sensitivity"
    return "error"


def parse_error_log(log_path: Path) -> dict:
    """Extract all counter values from a Parallax error.log."""
    result = {
        "above_bits":    0,
        "nan":           0,
        "inf":           0,
        "branch_flips":  0,
        "conv_errors":   0,
        "cond_detected": 0,
        "cancellation":  0,
        "sensitivity":   0,
        "untyped":       0,
        "suppressed":    0,
    }
    if not log_path.exists():
        return result

    patterns = {
        "above_bits":    r"Error above bits \d+ found (\d+)",
        "nan":           r"Total NaN found (\d+)",
        "inf":           r"Total Inf found (\d+)",
        "branch_flips":  r"Total branch flips found (\d+)",
        "conv_errors":   r"Total conversion errors found (\d+)",
        "cond_detected": r"Condition-number detections found (\d+)",
        "cancellation":  r"Total cancellation found (\d+)",
        "sensitivity":   r"Total sensitivity found (\d+)",
        "untyped":       r"Total untyped found (\d+)",
        "suppressed":    r"Total suppressed found (\d+)",
    }
    text = log_path.read_text(errors="replace")
    for key, pat in patterns.items():
        m = re.search(pat, text)
        if m:
            result[key] = int(m.group(1))
    return result


def parallax_category(counts: dict) -> str:
    if counts["inf"] > 0 or counts["nan"] > 0:
        if counts["sensitivity"] > 0:
            return "overflow-sensitivity"
        return "overflow"
    if counts["above_bits"] > 0:
        return "error"
    if counts["cancellation"] > 0:
        return "cancellation"
    if counts["sensitivity"] > 0:
        return "sensitivity"
    return "clean"


def is_flagged_eft(counts: dict) -> bool:
    return counts["above_bits"] > 0 or counts["nan"] > 0 or counts["inf"] > 0


def is_flagged_cond(counts: dict) -> bool:
    return is_flagged_eft(counts) or counts["cond_detected"] > 0


def collect_benchmarks() -> list[str]:
    names = set()
    for opt in OPT_LEVELS:
        opt_dir = PRODUCED_DIR / opt
        if opt_dir.is_dir():
            names.update(d.name for d in opt_dir.iterdir() if d.is_dir())
    return sorted(names)


def build_herbie_ground_truth(benchmarks: list[str]) -> dict[str, str]:
    gt = {}
    for name in benchmarks:
        tl = HERBIE_DIR / name / "timeline.json"
        gt[name] = herbie_category(tl) if tl.exists() else "unknown"
    return gt


def analyze_opt(opt: str, benchmarks: list[str],
                herbie_gt: dict[str, str]) -> dict:
    opt_dir = PRODUCED_DIR / opt
    results = {}

    for name in benchmarks:
        log = opt_dir / name / "error.log"
        counts   = parse_error_log(log)
        p_cat    = parallax_category(counts)
        h_cat    = herbie_gt.get(name, "unknown")
        results[name] = {
            "herbie":   h_cat,
            "parallax": p_cat,
            "counts":   counts,
            "flagged_eft":  is_flagged_eft(counts),
            "flagged_cond": is_flagged_cond(counts),
        }
    return results


def false_positive_analysis(results: dict, clean_names: set[str]) -> dict:
    eft_fp = 0; cond_fp = 0; total = 0
    fp_list = []
    for name in sorted(clean_names):
        if name not in results:
            continue
        r = results[name]
        total += 1
        if r["flagged_eft"]:
            eft_fp += 1
        if r["flagged_cond"]:
            cond_fp += 1
        if r["flagged_eft"] or r["flagged_cond"]:
            fp_list.append((name, r["flagged_eft"], r["flagged_cond"]))
    return {
        "total": total,
        "eft_fp": eft_fp,
        "cond_fp": cond_fp,
        "fp_list": fp_list,
        "eft_rate":  100 * eft_fp  / total if total else 0,
        "cond_rate": 100 * cond_fp / total if total else 0,
    }


def sensitivity_overflow_analysis(results: dict) -> dict:
    sens_names  = set()
    ovf_names   = set()
    reclassified = set()

    for name, r in results.items():
        c = r["counts"]
        if c["sensitivity"] > 0:
            sens_names.add(name)
        if c["inf"] > 0 or c["nan"] > 0:
            ovf_names.add(name)
        if r["parallax"] == "overflow-sensitivity":
            reclassified.add(name)

    both    = sens_names & ovf_names
    genuine = sens_names - ovf_names
    return {
        "sensitivity": sens_names,
        "overflow":    ovf_names,
        "both":        both,
        "genuine":     genuine,
        "reclassified": reclassified,
    }


def agreement_analysis(results: dict) -> dict:
    agreed = 0; total = 0; disagreed = []
    category_counts = defaultdict(int)

    for name, r in results.items():
        h = r["herbie"]; p = r["parallax"]
        if h == "unknown":
            continue
        total += 1
        p_norm = "overflow" if p == "overflow-sensitivity" else p
        if h == p_norm or (h == "clean" and p == "clean"):
            agreed += 1
            category_counts[h] += 1
        else:
            disagreed.append((name, h, p))

    return {
        "agreed": agreed,
        "total":  total,
        "rate":   100 * agreed / total if total else 0,
        "category_counts": dict(category_counts),
        "disagreed": disagreed,
    }


def write_opt_report(opt: str, results: dict,
                     clean_names: set[str]) -> None:
    out_path = SCRIPT_DIR / f"herbie/analysis_{opt}.txt"
    fp  = false_positive_analysis(results, clean_names)
    sa  = sensitivity_overflow_analysis(results)
    ag  = agreement_analysis(results)

    with open(out_path, "w") as f:
        f.write(f"Parallax Herbie Analysis — {opt}\n")
        f.write("=" * 60 + "\n\n")

        f.write("── Agreement with Herbie ─────────────────────────────\n")
        f.write(f"  Agreed:  {ag['agreed']} / {ag['total']}"
                f"  ({ag['rate']:.1f}%)\n")
        f.write("  Per category (agreed):\n")
        for cat, n in sorted(ag["category_counts"].items()):
            f.write(f"    {cat:<20} {n}\n")
        if ag["disagreed"]:
            f.write("  Disagreements:\n")
            for name, h, p in ag["disagreed"][:20]:
                f.write(f"    {name:<45}  herbie={h}  parallax={p}\n")
        f.write("\n")

        f.write("── False-Positive Analysis (Herbie-clean benchmarks) ─\n")
        f.write(f"  Clean benchmarks tested: {fp['total']}\n")
        f.write(f"  EFT-only   false positives: {fp['eft_fp']}"
                f"  ({fp['eft_rate']:.1f}%)\n")
        f.write(f"  EFT+cond   false positives: {fp['cond_fp']}"
                f"  ({fp['cond_rate']:.1f}%)\n")
        if fp["fp_list"]:
            f.write("  Flagged clean benchmarks:\n")
            for name, eft, cond in fp["fp_list"]:
                f.write(f"    {name:<45}  eft={int(eft)}  cond={int(cond)}\n")
        f.write("\n")

        f.write("── Sensitivity / Overflow Disambiguation ─────────────\n")
        f.write(f"  Sensitivity-flagged : {len(sa['sensitivity'])}\n")
        f.write(f"  Overflow-flagged    : {len(sa['overflow'])}\n")
        f.write(f"  Intersection        : {len(sa['both'])}\n")
        f.write(f"  Genuine sensitivity : {len(sa['genuine'])}\n")
        f.write(f"  Auto-reclassified   : {len(sa['reclassified'])}\n")
        if sa["genuine"]:
            f.write("  Genuine sensitivity benchmarks:\n")
            for n in sorted(sa["genuine"]):
                f.write(f"    {n}\n")
        if sa["both"]:
            f.write("  Reclassified (sens+overflow) benchmarks:\n")
            for n in sorted(sa["both"]):
                f.write(f"    {n}\n")
        f.write("\n")

    print(f"  Written: {out_path}")


def write_summary(all_results: dict[str, dict],
                  clean_names: set[str]) -> None:
    out_path = SCRIPT_DIR / "herbie/summary.txt"
    with open(out_path, "w") as f:
        f.write("Parallax Herbie Evaluation Summary\n")
        f.write("=" * 60 + "\n\n")
        f.write(f"{'Metric':<40} {'O0':>8} {'O1':>8} {'O2':>8}\n")
        f.write("-" * 68 + "\n")

        def row(label, fn):
            vals = [fn(all_results[opt]) for opt in OPT_LEVELS]
            f.write(f"  {label:<38} {vals[0]:>8} {vals[1]:>8} {vals[2]:>8}\n")

        row("Benchmarks analyzed",
            lambda r: str(len(r)))
        row("Agreement with Herbie",
            lambda r: f"{agreement_analysis(r)['agreed']}"
                      f"/{agreement_analysis(r)['total']}")
        row("Agreement rate",
            lambda r: f"{agreement_analysis(r)['rate']:.1f}%")
        row("EFT-only FP rate (clean set)",
            lambda r: f"{false_positive_analysis(r, clean_names)['eft_rate']:.1f}%")
        row("EFT+cond FP rate (clean set)",
            lambda r: f"{false_positive_analysis(r, clean_names)['cond_rate']:.1f}%")
        row("Sensitivity detections",
            lambda r: str(len(sensitivity_overflow_analysis(r)["sensitivity"])))
        row("Auto-reclassified (overflow-sens)",
            lambda r: str(len(sensitivity_overflow_analysis(r)["reclassified"])))
        row("Genuine sensitivity",
            lambda r: str(len(sensitivity_overflow_analysis(r)["genuine"])))

        f.write("\n")
        f.write("Clean benchmark list: scripts/benchmarks/herbie/clean_benchmarks.txt\n")
    print(f"  Written: {out_path}")


def main():
    print("Collecting benchmarks...")
    benchmarks = collect_benchmarks()
    if not benchmarks:
        print(f"ERROR: No benchmarks found under {PRODUCED_DIR}/O*/")
        sys.exit(1)
    print(f"  Found {len(benchmarks)} benchmarks")

    print("Building Herbie ground truth from timeline.json files...")
    herbie_gt = build_herbie_ground_truth(benchmarks)

    cats_path = SCRIPT_DIR / "herbie/herbie_categories.txt"
    with open(cats_path, "w") as f:
        for name in benchmarks:
            f.write(f"{name}\t{herbie_gt[name]}\n")
    print(f"  Written: {cats_path}")

    clean_names = {n for n, c in herbie_gt.items() if c == "clean"}
    clean_path  = SCRIPT_DIR / "herbie/clean_benchmarks.txt"
    with open(clean_path, "w") as f:
        for n in sorted(clean_names):
            f.write(n + "\n")
    print(f"  Clean benchmarks: {len(clean_names)} -> {clean_path}")

    print("Analyzing Parallax logs...")
    all_results: dict[str, dict] = {}
    for opt in OPT_LEVELS:
        print(f"  {opt}...")
        results = analyze_opt(opt, benchmarks, herbie_gt)
        all_results[opt] = results
        write_opt_report(opt, results, clean_names)

    print("Writing summary...")
    write_summary(all_results, clean_names)

    print("\nDone. Key files:")
    print(f"  {SCRIPT_DIR}/herbie/clean_benchmarks.txt")
    print(f"  {SCRIPT_DIR}/herbie/herbie_categories.txt")
    print(f"  {SCRIPT_DIR}/herbie/analysis_O{{0,1,2}}.txt")
    print(f"  {SCRIPT_DIR}/herbie/summary.txt")


if __name__ == "__main__":
    main()