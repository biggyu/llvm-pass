#!/usr/bin/env python3
"""Generate C (kernel + driver) from Herbie timeline.json fperrors.
Mode A (default): drive on worst-case input.
Mode B (--sample points.json): drive on Herbie's exact sampled points,
    with warmup + adaptive timed reps (in-process clock_gettime)."""

import json, re, sys, os, struct

# ---------- tokenizer ----------
def tokenize(s):
    s = re.sub(r'#s\(literal\s+(-?\d+)/(\d+)\s+\w+\)',
               lambda m: repr(float(m.group(1)) / float(m.group(2))), s)
    s = re.sub(r'#s\(literal\s+([-\d.eE+]+)\s+\w+\)', r'\1', s)
    return re.findall(r'\(|\)|[^\s()]+', s)

def parse_sexpr(tokens):
    tok = tokens.pop(0)
    if tok == '(':
        lst = []
        while tokens[0] != ')':
            lst.append(parse_sexpr(tokens))
        tokens.pop(0)
        return lst
    return tok

# ---------- op maps ----------
BINOPS = {'+':'+', '-':'-', '*':'*', '/':'/'}
FUNCS = {
    'sqrt':('sqrt','sqrtf'), 'cbrt':('cbrt','cbrtf'),
    'sin':('sin','sinf'), 'cos':('cos','cosf'), 'tan':('tan','tanf'),
    'asin':('asin','asinf'), 'acos':('acos','acosf'), 'atan':('atan','atanf'),
    'exp':('exp','expf'), 'exp2':('exp2','exp2f'), 'expm1':('expm1','expm1f'),
    'log':('log','logf'), 'log2':('log2','log2f'), 'log10':('log10','log10f'),
    'log1p':('log1p','log1pf'),
    'pow':('pow','powf'), 'fabs':('fabs','fabsf'), 'fma':('fma','fmaf'),
}
CONSTS = {'PI': 'M_PI', 'E': 'M_E'}
UNSUPPORTED_BY_PASS = {
    'fmod', 'log1p', 'hypot', 'copysign', 'atan2',
    'sinh', 'cosh', 'tanh', 'asinh', 'acosh', 'atanh',
    'floor', 'fmax', 'fmin', 'ceil', 'round', 'trunc',
}

def strip_suffix(op):
    m = re.match(r'^(.*)\.(f64|f32)$', op)
    return (m.group(1), m.group(2)) if m else (op, None)

def to_c(node, vars_seen, is_float):
    if isinstance(node, str):
        if re.match(r'^-?[\d.]+([eE][-+]?\d+)?$', node):
            return node
        vars_seen.add(node)
        return node
    op_raw = node[0]
    base, _ = strip_suffix(op_raw)
    if base in CONSTS and len(node) == 1:
        return CONSTS[base]
    if base in UNSUPPORTED_BY_PASS:
        raise ValueError(f"pass-unsupported op: {op_raw}")
    args = [to_c(a, vars_seen, is_float) for a in node[1:]]
    if base in BINOPS and len(args) == 2:
        return f"({args[0]} {BINOPS[base]} {args[1]})"
    if base == 'neg' and len(args) == 1:
        return f"(-{args[0]})"
    if base in FUNCS:
        fn = FUNCS[base][1] if is_float else FUNCS[base][0]
        return f"{fn}({', '.join(args)})"
    raise ValueError(f"unhandled op: {op_raw}")

def detect_precision(expr_str):
    has64, has32 = '.f64' in expr_str, '.f32' in expr_str
    if has32 and not has64: return 'f32'
    if has32 and has64: return 'mixed'
    return 'f64'

# ---------- fperrors ----------
def error_entries(fperrors):
    out = []
    for e in fperrors:
        expr = e[0]
        c1 = e[1] if len(e) > 1 else 0
        inputsA = e[3] if len(e) > 3 and isinstance(e[3], list) else None
        c3 = e[4] if len(e) > 4 else 0
        inputsB = e[5] if len(e) > 5 and isinstance(e[5], list) else None
        count = max(c1, c3)
        if count <= 0:
            continue
        worst = inputsB or inputsA
        out.append({"expr": expr, "count": count, "worst": worst})
    return out

def herbie_var_order(fperrors, vars_seen):
    order = []
    for e in fperrors:
        name = e[0]
        if name in vars_seen and name not in order:
            order.append(name)
    for v in sorted(vars_seen):
        if v not in order:
            order.append(v)
    return order

# ---------- points.json ----------
# ---------- points.json ----------
def bits_to_double(n):
    return struct.unpack(
        "<d",
        struct.pack("<Q", int(n) & 0xFFFFFFFFFFFFFFFF)
    )[0]

def load_points(points_path):
    with open(points_path, "r", encoding="utf-8") as f:
        pj = json.load(f)
    if not isinstance(pj, dict):
        raise ValueError("points.json must contain a JSON object")
    pvars = pj.get("vars", [])
    raw_points = pj.get("points", [])
    if not isinstance(pvars, list):
        raise ValueError("points.json field 'vars' must be a list")
    if not isinstance(raw_points, list):
        raise ValueError("points.json field 'points' must be a list")
    if not raw_points:
        raise ValueError("points.json contains no sample points")

    pts = []
    for row_index, row in enumerate(raw_points):
        if not isinstance(row, list):
            raise ValueError(
                f"points.json point {row_index} is not a list"
            )
        if len(row) != len(pvars):
            raise ValueError(
                f"points.json point {row_index} has {len(row)} values, "
                f"but {len(pvars)} variables were declared"
            )
        pts.append([bits_to_double(x) for x in row])
    return pvars, pts

# ---------- codegen ----------
def gen_kernel(top_expr, fperrors):
    prec = detect_precision(top_expr)
    is_float = (prec == 'f32')
    ctype = 'float' if is_float else 'double'
    ast = parse_sexpr(tokenize(top_expr))
    vars_seen = set()
    body = to_c(ast, vars_seen, is_float)
    vars_ = herbie_var_order(fperrors, vars_seen)
    return ctype, is_float, prec, body, vars_

def generate_worst(top_expr, worst, fperrors):
    ctype, is_float, prec, body, vars_ = gen_kernel(
        top_expr, fperrors
    )
    fmt = "%.9g" if is_float else "%.17g"
    suffix = "f" if is_float else ""
    # In C, kernel(void) explicitly means no parameters.
    params = (
        ", ".join(f"{ctype} {v}" for v in vars_)
        if vars_
        else "void"
    )
    if worst is None:
        arg_values = [f"1.0{suffix}" for _ in vars_]
    else:
        arg_values = [
            f"{float(x):.17e}{suffix}"
            for x in worst[:len(vars_)]
        ]

        # Supply fallback values if the worst-case input has fewer
        # values than the expression has variables.
        while len(arg_values) < len(vars_):
            arg_values.append(f"1.0{suffix}")
    args = ", ".join(arg_values)
    kernel_call = f"kernel({args})" if vars_ else "kernel()"

    code = f'''#include <stdio.h>
#include <math.h>

{ctype} kernel({params}) {{
    return {body};
}}

int main(void) {{
    volatile {ctype} r = {kernel_call};
    printf("{fmt}\\n", (double)r);
    return 0;
}}
'''

    return code, vars_, prec

def generate_sampled(
    top_expr,
    fperrors,
    pvars,
    points,
    warmup=3,
    target_evals=1_000_000
):
    ctype, is_float, prec, body, vars_ = gen_kernel(
        top_expr, fperrors
    )

    fmt = "%.9g" if is_float else "%.17g"
    suffix = "f" if is_float else ""

    nvars = len(vars_)
    npts = len(points)

    if npts == 0:
        raise ValueError("cannot generate sampled benchmark with zero points")

    # Keep approximately target_evals total kernel evaluations.
    reps = max(10, target_evals // npts)

    params = (
        ", ".join(f"{ctype} {v}" for v in vars_)
        if vars_
        else "void"
    )

    # ------------------------------------------------------------
    # Constant expression: no variables
    #
    # Do not generate:
    #     inputs[][0]
    #
    # The Herbie points still determine how many times the constant
    # expression is evaluated during each repetition.
    # ------------------------------------------------------------
    if nvars == 0:
        code = f'''#include <stdio.h>
#include <math.h>
#include <time.h>

{ctype} kernel(void) {{
    return {body};
}}

int main(void) {{
    const int npts = {npts};
    volatile {ctype} acc = 0;

    /* Warmup: untimed. */
    for (int w = 0; w < {warmup}; w++) {{
        for (int i = 0; i < npts; i++) {{
            acc += kernel();
        }}
    }}

    /* Time only the kernel loop. */
    double total_ms = 0.0;

    for (int r = 0; r < {reps}; r++) {{
        struct timespec t0;
        struct timespec t1;

        clock_gettime(CLOCK_MONOTONIC, &t0);

        for (int i = 0; i < npts; i++) {{
            acc += kernel();
        }}

        clock_gettime(CLOCK_MONOTONIC, &t1);

        total_ms += (t1.tv_sec - t0.tv_sec) * 1e3
                  + (t1.tv_nsec - t0.tv_nsec) / 1e6;
    }}

    printf("KERNEL_MS %.6f\\n", total_ms / {reps});
    printf("REPS {reps}\\n");
    printf("NPTS %d\\n", npts);
    printf("RESULT {fmt}\\n", (double)acc);

    return 0;
}}
'''

        return code, vars_, prec

    # ------------------------------------------------------------
    # Normal expression: one or more variables
    # ------------------------------------------------------------
    missing_vars = [v for v in vars_ if v not in pvars]

    if missing_vars:
        print(
            "warning: variables missing from points.json; "
            "using 1.0 for: "
            + ", ".join(missing_vars),
            file=sys.stderr,
        )

    indices = [
        pvars.index(v) if v in pvars else None
        for v in vars_
    ]

    rows = []

    for point_index, pt in enumerate(points):
        values = []

        for variable, source_index in zip(vars_, indices):
            if source_index is None:
                value = 1.0
            else:
                if source_index >= len(pt):
                    raise ValueError(
                        f"point {point_index} does not contain a value "
                        f"for variable {variable!r}"
                    )

                value = pt[source_index]

            values.append(f"{value:.17e}{suffix}")

        rows.append("    {" + ", ".join(values) + "},")

    table = "\n".join(rows)

    call_args = ", ".join(
        f"inputs[i][{index}]"
        for index in range(nvars)
    )

    code = f'''#include <stdio.h>
#include <math.h>
#include <time.h>

{ctype} kernel({params}) {{
    return {body};
}}

static const {ctype} inputs[{npts}][{nvars}] = {{
{table}
}};

int main(void) {{
    const int npts = {npts};
    volatile {ctype} acc = 0;

    /* Warmup: untimed. */
    for (int w = 0; w < {warmup}; w++) {{
        for (int i = 0; i < npts; i++) {{
            acc += kernel({call_args});
        }}
    }}

    /* Time only the kernel loop. */
    double total_ms = 0.0;

    for (int r = 0; r < {reps}; r++) {{
        struct timespec t0;
        struct timespec t1;

        clock_gettime(CLOCK_MONOTONIC, &t0);

        for (int i = 0; i < npts; i++) {{
            acc += kernel({call_args});
        }}

        clock_gettime(CLOCK_MONOTONIC, &t1);

        total_ms += (t1.tv_sec - t0.tv_sec) * 1e3
                  + (t1.tv_nsec - t0.tv_nsec) / 1e6;
    }}

    printf("KERNEL_MS %.6f\\n", total_ms / {reps});
    printf("REPS {reps}\\n");
    printf("NPTS %d\\n", npts);
    printf("RESULT {fmt}\\n", (double)acc);

    return 0;
}}
'''

    return code, vars_, prec

def main():
    # usage:
    #   parse_herbie.py <timeline.json> <out.c>
    #   parse_herbie.py <timeline.json> <out.c> --sample <points.json>
    tl_path, out_c = sys.argv[1], sys.argv[2]
    sample_mode = "--sample" in sys.argv
    points_path = sys.argv[sys.argv.index("--sample") + 1] if sample_mode else None

    timeline = json.load(open(tl_path))
    fperrors = None
    for phase in timeline:
        if isinstance(phase, dict) and "fperrors" in phase:
            fperrors = phase["fperrors"]; break
    if not fperrors:
        print(f"{tl_path}: NO_FPERRORS"); sys.exit(2)

    errs = error_entries(fperrors)
    if not errs:
        print(f"{tl_path}: CLEAN (no error)"); sys.exit(10)

    top_expr = fperrors[0][0]

    try:
        if sample_mode:
            pvars, points = load_points(points_path)
            code, vars_, prec = generate_sampled(
                top_expr,
                fperrors,
                pvars,
                points
            )
            mode = f"SAMPLED ({len(points)} points)"
        else:
            with_inputs = sorted(
                [e for e in errs if e["worst"] is not None],
                key=lambda x: x["count"],
                reverse=True
            )
            worst = with_inputs[0]["worst"] if with_inputs else None
            code, vars_, prec = generate_worst(
                top_expr,
                worst,
                fperrors
            )
            mode = "WORST-CASE"

    except (ValueError, KeyError, TypeError, json.JSONDecodeError) as ex:
        print(f"{tl_path}: GENERATION_ERROR {ex}")
        sys.exit(4)

    os.makedirs(os.path.dirname(out_c) or ".", exist_ok=True)
    with open(out_c, "w") as f:
        f.write(code)
    print(f"wrote {out_c} [{prec}] {mode}")
    print(f"  vars={vars_}")

if __name__ == "__main__":
    main()