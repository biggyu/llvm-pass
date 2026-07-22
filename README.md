# Efficiently combining error-free transformations and condition numbers for floating-point debugging tools

An LLVM instrumentation pass that combines EFT shadow execution with condition-number analysis to detect numerical errors and classify their mechanism (catastrophic cancellation vs. input sensitivity)

Build on the shadow-execution approach of [EFTSanitizer](https://github.com/rutgers-apl/EFTSanitizer), extended with the condition-number error classification method of [arXiv:2503.11884 [math.NA]](https://arxiv.org/abs/2503.11884v1).

## Quick Start

### Installing

1. Install Prerequisites:
```
    apt-get update
    apt-get install -y build-essential cmake libgmp3-dev libmpfr-dev python3
```

2. Install LLVM 21. The pass uses the new pass manager and must be built against the same LLVM version as the `opt` used to run it.
```
    apt-get install -y clang-21 llvm-21-dev
```

    Verify:
```
    clang-21 --version
    opt-21 --version
```

3. Set the following environment variables (only needed if LLVM 21 is not system default):

```
    export LLVM_DIR="<path/to/LLVM/dir>"
    export LLVM_OPT="$LLVM_DIR/bin/opt"
    export LLVM_CLANG="$LLVM_DIR/bin/clang"
    export LLVM_CLANGXX="$LLVM_DIR/bin/clang++"
```

4. Clone the repo:
```
    git clone https://github.com/biggyu/llvm-pass
    cd llvm-pass
```

5. Build the pass and runtime:
```
    sh scripts/build.sh <PROFILING> <DEBUG> <OPT>
```

### Options

| Option | Default | Description |
|---|---|---|
| `PROFILING` | 0(Off) | Profiling each function call count and total time |
| `DEBUG` | 0(Off) | Generate error.log |
| `OPT` | 0 | Compiler optimization level |

    This produces:
    - `build/passes/ShadowMem/ShadowMem.so` - the instrumentation pass
    - `build/runtime/libpass_runtime.a` - the runtime library

6. Run the controlled test suite to verify the installation:

```
    sh scripts/controlled_test.sh
    sh scripts/controlled_cmp.sh
```

This is the following output:

```
benchmark              result   failures
---------              ------   --------
cancel_sqrt            PASS
clean_poly             PASS
log_one                PASS
nan_inf                PASS
sin_near_zero          PASS
----
controlled: 5 passed, 0 failed
```

If all five pass, the pass, runtime, and toolchain are correctly configured.

## Instrumenting an application

Compile the application to LLVM IR with the desired optimization and no FMA fusion, run the pass, then link against the runtime.

```
    $ sh scripts/run.sh ShadowMem shadowmem <test file> <DEBUG> <OPT>
```

### Reading the output

Results are written to ./examples/error.log

```
Error above bits 50 found 12
Total NaN found 0
Total Inf found 3
Total branch flips found 0
Total conversion errors found 0

Condition-number detections found 12
Total cancellation found 8
Total sensitivity found 4
Total untyped found 0
Total suppressed found 0
```

The first five lines are EFT/MPFR error report (operations exceeding the bit-error threshold, exceptions, branch flips, conversion errors). The remaining lines are the condition-number classification: how many detections were attributed to catastrophic cancellation, to input sensitivity, left untyped, or suppressed because all operands were exact.

## Scripts

| Script | Purpose |
|---|---|
| `scripts/build.sh` | build the pass and runtime |
| `scripts/controlled_test.sh` | run the 5 hand-written correctness cases |
| `scripts/controlled_cmp.sh` | compare controlled results against expected |
| `scripts/parse_herbie.py` | generate C from a Herbie `timeline.json` |
| `scripts/herbie.sh` | run the Herbie arith25 suite |
| `benchmarks/herbie-arith25/comp_herbie.py` | compare results against Herbie's `FpError` |
| `scripts/polybench.sh` | run the PolyBench suite |

## Reproducing the evaluation

## Benchmark Suites

This repository includes validation and performance scripts for the Herbie `arith25` benchmark suite and PolyBench/C.

---

## Herbie `arith25` Suite

The Herbie suite compares the pass's reported floating-point errors against Herbie's `FpError` analysis, which serves as a high-precision reference.

### Obtain the Herbie Reports

The reports must be generated from the `arith25` tag of [Herbie](https://github.com/herbie-fp/herbie):

```sh
git clone https://github.com/herbie-fp/herbie
cd herbie
git checkout tags/arith25
```

For each benchmark, place the resulting `timeline.json` and `points.json` files under:

```text
benchmarks/herbie-arith25/report/<benchmark>/
```

### Run the Validation

```sh
# Correctness: run each benchmark using Herbie's worst-case inputs
sh scripts/herbie_single.sh <MAX> worst <OPT>

# Timing: run each benchmark using Herbie's sampled inputs
sh scripts/herbie_single.sh <MAX> sample <OPT>

# Run both sample & worst on O0, O1, and O2
sh scripts/herbie.sh
```

| Argument | Default | Description |
|---|---:|---|
| `MAX` | `5` | Maximum number of benchmarks to run. Use `0` to run all benchmarks. |
| `MODE` | `sample` | Input mode: `worst` or `sample`. |
| `OPT` | `0` | Compiler optimization level: `0`, `1`, or `2`. |

Examples:

```sh
# Run all benchmarks in correctness mode at O0
sh scripts/herbie_single.sh 0 worst 0

# Run the first 50 benchmarks in timing mode at O2
sh scripts/herbie_single.sh 50 sample 2
```

### Herbie Outputs

Results are written to:

- `benchmarks/herbie-arith25/src/` — generated C sources
- `benchmarks/herbie-arith25/expected/<benchmark>/` — reference build artifacts and logs
- `benchmarks/herbie-arith25/produced/timing_O<OPT>.csv` — timing results for `sample` mode
- `benchmarks/herbie-arith25/produced/compare_<MODE>_O<OPT>.log` — comparison of herbie vs results
- `benchmarks/herbie-arith25/produced/<benchmark>/error.log` — pass output for each benchmark

### Compare Against Herbie

```sh
python3 scripts/comp_herbie.py \
    benchmarks/herbie-arith25/report \
    benchmarks/herbie-arith25/produced
```

Example summary:

```text
Summary:
  AGREE_CANCEL       64
  AGREE_ERROR       137
  AGREE_OVERFLOW    118
  AGREE_SENSITIVITY  16
  NO_PROD             20
```

The categories have the following meanings:

| Category | Meaning |
|---|---|
| `AGREE_CANCEL` | Both tools detected an issue classified as cancellation. |
| `AGREE_ERROR` | Both tools detected a floating-point error without a more specific classification. |
| `AGREE_OVERFLOW` | Both tools detected an overflow-related issue. |
| `AGREE_SENSITIVITY` | Both tools detected sensitivity or condition-number-related behavior. |
| `NO_PROD` | No corresponding pass output was produced, commonly because the benchmark uses unsupported operations such as `atan2`, hyperbolic functions, `floor`, or `fmax`. |

The exact counts may change when the pass, benchmark generator, supported operations, or classification logic changes.

### Herbie Notes

- `worst` mode is intended for correctness comparison.
- `sample` mode is intended for timing and broader execution coverage.
- Some benchmarks may be skipped or fail when they use operations that are not currently instrumented.
- Error logs and generated artifacts may be large and should usually remain outside version control unless they are required as reference results.

---

## PolyBench/C

The PolyBench script compiles and runs the PolyBench/C kernels with both the uninstrumented baseline and the instrumented LLVM pass.

### Prerequisites

The following tools and files are required:

- `clang`
- `clang++`
- `opt`
- `timeout`
- MPFR and GMP development libraries
- `build/passes/ShadowMem/ShadowMem.so`
- `build/runtime/libpass_runtime.a`
- `benchmarks/polybench/utilities/polybench.c`
- `benchmarks/polybench/utilities/polybench.h`

The script is POSIX-shell compatible and may be executed with `sh`.

### Run PolyBench

```sh
sh scripts/polybench_single.sh <DATASET> <OPT> <MAX>

# Runs all dataset, opt
sh scripts/polybench.sh
```

| Argument | Default | Description |
|---|---:|---|
| `DATASET` | `SMALL` | Dataset size: `MINI`, `SMALL`, `MEDIUM`, or `LARGE`. |
| `OPT` | `0` | Compiler optimization level: `0`, `1`, or `2`. |
| `MAX` | `0` | Maximum number of benchmarks to run. Use `0` to run all benchmarks. |

Examples:

```sh
# Run all MINI benchmarks at O0
sh scripts/polybench_single.sh MINI 0 0

# Run the first 10 MEDIUM benchmarks at O2
sh scripts/polybench_single.sh MEDIUM 2 10
```

### Baseline and Instrumented Repetitions

The baseline and instrumented binaries use separate repetition counts because the baseline kernels are often too short to time reliably, while the instrumented kernels are substantially slower.

The repetition counts may be overridden through environment variables:

```sh
sh scripts/polybench_single.sh MINI 0 0
```

| Variable | Description |
|---|---|
| `FPCHECK_THRESHOLD` | Reporting threshold passed to the runtime. The script default is `1e15`. |
| `LLVM_CLANG` | Path or command name for `clang`. |
| `LLVM_CLANGXX` | Path or command name for `clang++`. |
| `LLVM_OPT` | Path or command name for `opt`. |

The baseline and instrumented repetition counts do not need to match. Overhead is calculated using the average time per kernel repetition:

```text
baseline_average     = baseline_total / baseline_reps
instrumented_average = instrumented_total / instrumented_reps

overhead_ratio = instrumented_average / baseline_average
```

### PolyBench Outputs

Results are written to:

- `benchmarks/polybench/out/<benchmark>/` — executables, LLVM IR, compiler logs, and timing output
- `benchmarks/polybench/produced/<benchmark>/error.log` — pass output for each benchmark
- `benchmarks/polybench/produced/polybench_<DATASET>_O<OPT>.csv` — combined timing results

The timing CSV contains:

| Column | Description |
|---|---|
| `benchmark` | PolyBench kernel name |
| `dataset` | Dataset size |
| `opt` | Compiler optimization level |
| `base_reps` | Baseline repetition count |
| `inst_reps` | Instrumented repetition count |
| `baseline_total_s` | Total measured baseline time |
| `baseline_avg_s` | Baseline time per repetition |
| `instrumented_total_s` | Total measured instrumented time |
| `instrumented_avg_s` | Instrumented time per repetition |
| `overhead_ratio` | Instrumented average divided by baseline average |
| `instr_status` | Instrumented execution status, such as `ok`, `timeout`, `crash`, or `no_timing` |

<!-- ### Timing Notes and Limitations

- Very small `MINI` and `SMALL` kernels may remain below the reliable resolution of the PolyBench timer, even with many repetitions.
- An average time that is physically implausible, such as a fraction of a nanosecond for an entire kernel, indicates that the measurement should not be used for overhead analysis.
- `MEDIUM` and `LARGE` datasets are preferred for final performance measurements because they reduce timer-resolution noise without requiring extremely large repetition counts.
- Some PolyBench kernels modify their input arrays in place. Repeating such kernels without reinitializing their inputs does not execute the same workload each time.
- Decomposition kernels such as `cholesky`, `gramschmidt`, `lu`, and `ludcmp` may produce invalid or non-finite values when repeatedly applied to already-modified data.
- For destructive kernels, use one repetition, reinitialize inputs between repetitions, or use a larger dataset.
- A segmentation fault in an instrumented run is recorded as `crash`; a run exceeding the timeout is recorded as `timeout`.
- The instrumented runtime should still handle `NaN`, infinity, and division by zero without crashing, even when a benchmark produces those values.

### Recommended Performance Methodology

For final timing results:

1. Prefer `MEDIUM` or `LARGE` datasets.
2. Use enough baseline repetitions to obtain a measurable total time.
3. Use fewer instrumented repetitions when instrumentation overhead is high.
4. Record both total and average time.
5. Calculate overhead from average time per repetition.
6. Run multiple independent trials and report the median.
7. Do not report overhead when the baseline timing is below the timer's reliable resolution.
8. Document any benchmark-specific repetition overrides or crashes. -->

---

## Generated Files and Version Control

Generated executables, LLVM IR, timing files, and error logs can be large and machine-specific. Unless reference outputs are intentionally maintained in the repository, consider excluding the following paths with `.gitignore`:

```gitignore
benchmarks/herbie-arith25/expected/
benchmarks/herbie-arith25/produced/
benchmarks/polybench/out/
benchmarks/polybench/produced/
```

Keep empty directories in version control with `.gitkeep` only when the repository structure requires them.


