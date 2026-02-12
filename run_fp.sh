cd build
make
cd ../
$LLVM_CLANG -emit-llvm -c ./inputs/input_for_fp.c -o inputs/input_for_fp.ll
$LLVM_OPT -load-pass-plugin ./build/libFPPrinter.so --passes="print<fp>" -disable-output ./inputs/input_for_fp.ll
