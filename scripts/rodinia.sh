#!/usr/bin/env bash
set -u

R="benchmarks/rodinia/openmp"
DATA="benchmarks/rodinia/data"
B="scripts/rodinia/build_bench.sh"

rm -rf ./build
sh ./scripts/build.sh 0 1 0

echo "########## Rodinia benchmarks ##########"

# each line: build_bench <name> <src_dir> <run args...>
# bash "$B" hotspot "$R/hotspot" 64 64 2 1 "$DATA/hotspot/temp_512" "$DATA/hotspot/power_512" output.out
# bash "$B" kmeans  "$R/kmeans/kmeans_serial" -i "$DATA/kmeans/100"
# bash "$B" backprop "$R/backprop" 65536
# bash "$B" nn "$R/nn" "$R/nn/filelist_4" 5 30 90
# bash "$B" lavaMD "$R/lavaMD" -boxes1d 10
# bash ./scripts/rodinia/run_myocyte.sh myocyte "$R/myocyte" 100 1 0 4
bash "$B" particlefilter "$R/particlefilter" -x 128 -y 128 -z 10 -np 1000

echo
echo "=== Rodinia results ==="
for log in tests/rodinia/produced/rodinia-*.log; do
    [ -e "$log" ] || continue
    echo "--- $(basename "$log") ---"; head -6 "$log"; echo
done
