
for thresh in 1e3 1e6; do
# for thresh in 1e3 1e6 1e9 1e12 1e15; do
    FPCHECK_THRESHOLD=$thresh \
    FPCHECK_BITS=50 \
    OPT=O0 \
    sh scripts/benchmarks/herbie.sh
    
    python scripts/benchmarks/analyze_herbie.py --label "T_kappa=$thresh"
done