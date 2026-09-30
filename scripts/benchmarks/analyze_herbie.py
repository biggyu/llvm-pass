
import json, os, re, sys, argparse
from pathlib import Path
from collections import defaultdict

PROJECT_ROOT = Path(".")
SCRIPT_DIR   = PROJECT_ROOT / "scripts" / "benchmarks" / "herbie"
HERBIE_DIR   = PROJECT_ROOT / "benchmarks" / "herbie-arith25" / "report"
PRODUCED_DIR = PROJECT_ROOT / "benchmarks" / "herbie-arith25" / "produced" / "sample"
OPT_LEVELS   = ["O0"]
# OPT_LEVELS   = ["O0", "O1", "O2"]

SCRIPT_DIR.mkdir(parents=True, exist_ok=True)

def herbie_reference(timeline_path: Path) -> dict | None:
    try:
        with open(timeline_path) as f:
            tl = json.load(f)
    except Exception:
        return None

    fperrors = None
    for phase in tl:
        if isinstance(phase, dict) and "fperrors" in phase:
            fperrors = phase["fperrors"]
            break
    if fperrors is None:
        return None

    error_ops = []
    for e in fperrors:
        expr    = e[0]
        c1      = e[1] if len(e) > 1 else 0
        inputsA = e[3] if len(e) > 3 and isinstance(e[3], list) else None
        c3      = e[4] if len(e) > 4 else 0
        inputsB = e[5] if len(e) > 5 and isinstance(e[5], list) else None
        count   = max(c1, c3)
        if count <= 0:
            continue
        worst = inputsB or inputsA
        error_ops.append({"expr": expr, "count": count, "worst": worst})

    if not error_ops:
        return {
            "has_error": False, "top_expr": None, "top_count": 0,
            "error_ops": [], "likely_overflow": False,
            "likely_cancel": False, "category": "clean",
        }

    top   = max(error_ops, key=lambda o: o["count"])
    expr  = top["expr"]
    worst = top["worst"]

    big                = bool(worst) and any(abs(float(x)) > 1e150 for x in worst)
    has_sub            = "-.f64" in expr or "-.f32" in expr
    has_div            = "/.f64" in expr or "/.f32" in expr
    has_mul            = "*.f64" in expr or "*.f32" in expr
    has_exp            = "exp" in expr
    has_trig           = any(op in expr for op in
                             ["sin", "cos", "tan", "asin", "acos", "atan"])
    has_log            = "log" in expr
    likely_overflow    = big and (has_div or has_mul or has_exp)
    likely_cancel      = has_sub or has_log
    likely_sensitivity = has_exp or has_trig

    if likely_overflow:
        category = "overflow"
    elif likely_cancel:
        category = "cancellation"
    elif likely_sensitivity:
        category = "sensitivity"
    else:
        category = "error"

    return {
        "has_error":          True,
        "top_expr":           expr,
        "top_count":          top["count"],
        "error_ops":          error_ops,
        "likely_overflow":    likely_overflow,
        "likely_cancel":      likely_cancel,
        "category":           category,
    }


# ── Parallax log parsing ──────────────────────────────────────────────────────
def parse_error_log(log_path: Path) -> dict | None:
    if not log_path.exists():
        return None
    txt = log_path.read_text(errors="replace")

    def num(pattern):
        m = re.findall(pattern + r'.*?(\d+)', txt)
        return int(m[0]) if m else 0

    return {
        "above_thres":   num(r"Error above bits \d+ found"),
        "nan":           num(r"Total NaN found"),
        "inf":           num(r"Total Inf found"),
        "branch_flips":  num(r"Total branch flips found"),
        "conv_errors":   num(r"Total conversion errors found"),
        "cond_detected": num(r"Condition-number detections found"),
        "cancellation":  num(r"Total cancellation found"),
        "sensitivity":   num(r"Total sensitivity found"),
        "untyped":       num(r"Total untyped found"),
        "suppressed":    num(r"Total suppressed found"),
    }


def parallax_category(p: dict) -> str:
    if p["inf"] > 0 or p["nan"] > 0:
        if p["sensitivity"] > 0:
            return "overflow-sensitivity"
        return "overflow"
    if p["above_thres"] > 0:
        return "error"
    if p["cancellation"] > 0:
        return "cancellation"
    if p["sensitivity"] > 0:
        return "sensitivity"
    return "clean"


def your_error_fired(p: dict) -> bool:
    return (p["above_thres"] > 0 or p["cond_detected"] > 0
            or p["nan"] > 0 or p["inf"] > 0)


def classify(ref: dict | None, prod: dict | None) -> tuple[str, str]:
    if ref is None:
        return "NO_REF", "no herbie timeline"
    if prod is None:
        return "NO_PROD", "no produced error.log"

    herbie_error = ref["has_error"]
    your_error   = your_error_fired(prod)

    if not herbie_error and not your_error:
        return "AGREE_CLEAN", "both clean"
    if herbie_error and your_error:
        if ref["likely_overflow"] and (prod["inf"] > 0 or prod["nan"] > 0):
            return "AGREE_OVERFLOW", "both: overflow/inf"
        if prod["sensitivity"] > 0 and prod["cancellation"] == 0:
            return "AGREE_SENSITIVITY", "condition: sensitivity"
        if ref["likely_cancel"] and prod["cancellation"] > 0:
            return "AGREE_CANCEL", "both: cancellation"
        return "AGREE_ERROR", "both found error"
    if herbie_error and not your_error:
        if ref["likely_overflow"]:
            return "MISS_OVERFLOW", "herbie overflow, parallax silent"
        return "MISS", "herbie error, parallax silent"
    return "FALSE_POS", "parallax flagged, herbie clean"


def collect_benchmarks() -> list[str]:
    names = set()
    for opt in OPT_LEVELS:
        d = PRODUCED_DIR / opt
        if d.is_dir():
            for p in d.iterdir():
                if p.is_dir() and (p / "error.log").exists():
                    if (HERBIE_DIR / p.name / "timeline.json").exists():
                        names.add(p.name)
    return sorted(names)


def build_ground_truth(benchmarks: list[str]) -> dict[str, dict | None]:
    return {
        name: herbie_reference(HERBIE_DIR / name / "timeline.json")
        if (HERBIE_DIR / name / "timeline.json").exists() else None
        for name in benchmarks
    }


def analyze_opt(opt: str, benchmarks: list[str],
                gt: dict) -> dict[str, dict]:
    results = {}
    opt_dir = PRODUCED_DIR / opt
    for name in benchmarks:
        log   = opt_dir / name / "error.log"
        prod  = parse_error_log(log)
        ref   = gt.get(name)
        verdict, note = classify(ref, prod)
        results[name] = {
            "ref":     ref,
            "prod":    prod,
            "verdict": verdict,
            "note":    note,
            "h_cat":   ref["category"] if ref else "unknown",
            "p_cat":   parallax_category(prod) if prod else "no_log",
        }
    return results


def compute_stats(results: dict, clean_names: set[str]) -> dict:
    tally      = defaultdict(int)
    fp_eft     = 0; fp_cond = 0; fp_total = 0
    sens_names = set(); ovf_names = set(); reclass = set()

    for name, r in results.items():
        tally[r["verdict"]] += 1
        p = r["prod"]
        if p is None:
            continue
        if name in clean_names:
            fp_total += 1
            if p["above_thres"] > 0 or p["nan"] > 0 or p["inf"] > 0:
                fp_eft += 1
            if your_error_fired(p):
                fp_cond += 1
        if p["sensitivity"] > 0:
            sens_names.add(name)
        if p["inf"] > 0 or p["nan"] > 0:
            ovf_names.add(name)
        if r["p_cat"] == "overflow-sensitivity":
            reclass.add(name)

    agreed = sum(v for k, v in tally.items() if k.startswith("AGREE"))
    total  = sum(tally.values())
    return {
        "tally":        dict(tally),
        "agreed":       agreed,
        "total":        total,
        "agree_rate":   100 * agreed / total if total else 0,
        "fp_total":     fp_total,
        "fp_eft":       fp_eft,
        "fp_cond":      fp_cond,
        "fp_eft_rate":  100 * fp_eft  / fp_total if fp_total else 0,
        "fp_cond_rate": 100 * fp_cond / fp_total if fp_total else 0,
        "sens":         sens_names,
        "ovf":          ovf_names,
        "both":         sens_names & ovf_names,
        "genuine_sens": sens_names - ovf_names,
        "reclass":      reclass,
    }


def write_opt_report(opt: str, results: dict, stats: dict,
                     clean_names: set[str], out_dir: Path) -> None:
    out = out_dir / f"analysis_{opt}.txt"
    with open(out, "w") as f:
        f.write(f"Parallax vs Herbie — {opt}\n")
        f.write("=" * 70 + "\n\n")

        f.write("── Agreement ──\n")
        f.write(f"  Total benchmarks : {stats['total']}\n")
        f.write(f"  Agreed           : {stats['agreed']} "
                f"({stats['agree_rate']:.1f}%)\n")
        f.write("  Verdict breakdown:\n")
        for k, v in sorted(stats["tally"].items()):
            f.write(f"    {k:<22} {v}\n")
        f.write("\n")

        misses = [(n, r) for n, r in results.items()
                  if r["verdict"] in ("MISS", "MISS_OVERFLOW")]
        fps    = [(n, r) for n, r in results.items()
                  if r["verdict"] == "FALSE_POS"]

        if misses:
            f.write("── Misses ───\n")
            for name, r in sorted(misses):
                f.write(f"  {name:<45}  [{r['verdict']}] {r['note']}\n")
            f.write("\n")
        if fps:
            f.write("── False positives ───\n")
            for name, r in sorted(fps):
                f.write(f"  {name:<45}"
                        f"  eft={int(r['prod']['above_thres'] > 0)}"
                        f"  cond={int(r['prod']['cond_detected'] > 0)}\n")
            f.write("\n")

        f.write("── False-positive rate on Herbie-clean benchmarks ───\n")
        f.write(f"  Clean tested  : {stats['fp_total']}\n")
        f.write(f"  EFT-only FP   : {stats['fp_eft']} ({stats['fp_eft_rate']:.1f}%)\n")
        f.write(f"  EFT+cond FP   : {stats['fp_cond']} ({stats['fp_cond_rate']:.1f}%)\n\n")

        f.write("── Sensitivity / Overflow disambiguation ───\n")
        f.write(f"  Sensitivity-flagged : {len(stats['sens'])}\n")
        f.write(f"  Overflow-flagged    : {len(stats['ovf'])}\n")
        f.write(f"  Intersection        : {len(stats['both'])}\n")
        f.write(f"  Genuine sensitivity : {len(stats['genuine_sens'])}\n")
        f.write(f"  Auto-reclassified   : {len(stats['reclass'])}\n\n")

        f.write("── Per-benchmark detail ──\n")
        f.write(f"  {'benchmark':<45} {'herbie':<14} {'parallax':<22} verdict\n")
        f.write("  " + "-" * 100 + "\n")
        for name in sorted(results):
            r = results[name]
            f.write(f"  {name:<45} {r['h_cat']:<14} {r['p_cat']:<22} {r['verdict']}\n")

    print(f"  Written: {out}")


def write_summary(all_results: dict, all_stats: dict,
                  out_dir: Path, label: str = "") -> None:
    out = out_dir / "summary.txt"
    opts = OPT_LEVELS
    with open(out, "w") as f:
        header = "Parallax Herbie Evaluation Summary"
        if label:
            header += f" [{label}]"
        f.write(header + "\n")
        f.write("=" * 70 + "\n\n")

        thresh = os.environ.get("FPCHECK_THRESHOLD", "1e15")
        bits   = os.environ.get("FPCHECK_BITS", "50")
        f.write(f"  Configuration: FPCHECK_THRESHOLD={thresh}"
                f"  FPCHECK_BITS={bits}\n\n")

        def row(label_str, vals):
            f.write(f"  {label_str:<42}")
            for v in vals:
                f.write(f" {str(v):>10}")
            f.write("\n")

        f.write(f"  {'Metric':<42}")
        for o in opts:
            f.write(f" {o:>10}")
        f.write("\n")
        f.write("  " + "-" * (42 + 11 * len(opts)) + "\n")

        row("Benchmarks analyzed",
            [all_stats[o]["total"] for o in opts])
        row("Agreement with Herbie",
            [f"{all_stats[o]['agreed']}/{all_stats[o]['total']}" for o in opts])
        row("Agreement rate",
            [f"{all_stats[o]['agree_rate']:.1f}%" for o in opts])
        row("EFT-only FP rate (clean set)",
            [f"{all_stats[o]['fp_eft_rate']:.1f}%" for o in opts])
        row("EFT+cond FP rate (clean set)",
            [f"{all_stats[o]['fp_cond_rate']:.1f}%" for o in opts])
        row("Sensitivity detections",
            [len(all_stats[o]["sens"]) for o in opts])
        row("Auto-reclassified (ovf-sens)",
            [len(all_stats[o]["reclass"]) for o in opts])
        row("Genuine sensitivity",
            [len(all_stats[o]["genuine_sens"]) for o in opts])

        f.write("\n")
        for opt in opts:
            f.write(f"  Verdict breakdown {opt}:\n")
            for k, v in sorted(all_stats[opt]["tally"].items()):
                f.write(f"    {k:<22} {v}\n")
            f.write("\n")

    print(f"  Written: {out}")


def write_sweep_table(sweep_dir: Path) -> None:
    """
    Compile a threshold sweep table from all labeled subdirectories.
    Run after completing all labeled threshold runs.
    """
    rows = []
    for sub in sorted(sweep_dir.iterdir()):
        if not sub.is_dir():
            continue
        summary = sub / "summary.txt"
        if not summary.exists():
            continue
        txt = summary.read_text()
        thresh_m = re.search(r"FPCHECK_THRESHOLD=([\d.e+]+)", txt)
        agree_m  = re.search(
            r"Agreement rate\s+([\d.]+%)\s+([\d.]+%)\s+([\d.]+%)", txt)
        fp_m     = re.search(
            r"EFT\+cond FP rate.*?([\d.]+%)\s+([\d.]+%)\s+([\d.]+%)", txt)
        cancel_m = re.search(
            r"AGREE_CANCEL\s+(\d+)", txt)
        miss_m   = re.search(
            r"MISS\b.*?(\d+).*?MISS_OVERFLOW.*?(\d+)", txt, re.DOTALL)
        if thresh_m and agree_m:
            rows.append({
                "thresh":    thresh_m.group(1),
                "agree_O0":  agree_m.group(1),
                "agree_O1":  agree_m.group(2),
                "agree_O2":  agree_m.group(3),
                "fp_O0":     fp_m.group(1) if fp_m else "?",
                "cancel":    cancel_m.group(1) if cancel_m else "?",
            })

    if not rows:
        print("No labeled subdirectories with summary.txt found.")
        return

    out = sweep_dir / "threshold_sweep_summary.txt"
    with open(out, "w") as f:
        f.write("Condition-Number Threshold Sweep (T_EFT=50 bits fixed)\n")
        f.write("=" * 70 + "\n")
        f.write(f"  {'T_kappa':<14} {'Agree O0':>10} {'Agree O1':>10}"
                f" {'Agree O2':>10} {'FP O0':>10} {'Cancel':>10}\n")
        f.write("  " + "-" * 65 + "\n")
        for r in rows:
            f.write(f"  {r['thresh']:<14} {r['agree_O0']:>10} {r['agree_O1']:>10}"
                    f" {r['agree_O2']:>10} {r['fp_O0']:>10} {r['cancel']:>10}\n")
    print(f"  Sweep table written: {out}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--label", default="")
    parser.add_argument(
        "--sweep-table", action="store_true",
        help="Compile threshold sweep table from all labeled subdirectories.")
    args = parser.parse_args()

    if args.sweep_table:
        write_sweep_table(SCRIPT_DIR)
        return

    out_dir = SCRIPT_DIR / args.label if args.label else SCRIPT_DIR
    print(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    print("Collecting benchmarks...")
    benchmarks = collect_benchmarks()
    if not benchmarks:
        print(f"ERROR: no benchmarks found under {PRODUCED_DIR}/O*/")
        sys.exit(1)
    print(f"  {len(benchmarks)} benchmarks found")

    print("Parsing Herbie ground truth...")
    gt = build_ground_truth(benchmarks)
    known = sum(1 for v in gt.values() if v is not None)
    print(f"  {known}/{len(benchmarks)} have a timeline.json")

    # Shared files always go to SCRIPT_DIR
    cats_path = SCRIPT_DIR / "herbie_categories.txt"
    with open(cats_path, "w") as f:
        for name in benchmarks:
            ref = gt.get(name)
            cat = ref["category"] if ref else "unknown"
            f.write(f"{name}\t{cat}\n")
    print(f"  Written: {cats_path}")

    clean_names = {n for n, r in gt.items()
                   if r is not None and not r["has_error"]}
    clean_path  = SCRIPT_DIR / "clean_benchmarks.txt"
    with open(clean_path, "w") as f:
        for n in sorted(clean_names):
            f.write(n + "\n")
    print(f"  Clean benchmarks: {len(clean_names)} -> {clean_path}")

    # Per-opt analysis
    all_results: dict[str, dict] = {}
    all_stats:   dict[str, dict] = {}
    for opt in OPT_LEVELS:
        print(f"Analyzing {opt}...")
        results = analyze_opt(opt, benchmarks, gt)
        stats   = compute_stats(results, clean_names)
        all_results[opt] = results
        all_stats[opt]   = stats
        write_opt_report(opt, results, stats, clean_names, out_dir)
        print(f"  Agreement: {stats['agreed']}/{stats['total']}"
              f"  ({stats['agree_rate']:.1f}%)"
              f"  FP: {stats['fp_cond']}/{stats['fp_total']}"
              f"  ({stats['fp_cond_rate']:.1f}%)")

    write_summary(all_results, all_stats, out_dir, args.label)
    print(f"\nDone. Results in: {out_dir}")


if __name__ == "__main__":
    main()