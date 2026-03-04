cd build
rm -rf *
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../FPPrinter
make
cd ../
if [ "$1" -eq "1" ]; then
    $LLVM_CLANG -O0 -emit-llvm -c ./inputs/input_for_fp.c -o ./inputs/input_for_fp.bc
    # Run the pass through opt
    $LLVM_OPT -load-pass-plugin ./build/libFPPrinter.so --passes="print<fp>-injected" ./inputs/input_for_fp.bc -o instrumented.bin
    $LLVM_DIR/bin/lli instrumented.bin
else
    $LLVM_CLANG -emit-llvm -c ./inputs/input_for_fp.c -o inputs/input_for_fp.ll
    $LLVM_OPT -load-pass-plugin ./build/libFPPrinter.so --passes="print<fp>" -disable-output ./inputs/input_for_fp.ll
fi