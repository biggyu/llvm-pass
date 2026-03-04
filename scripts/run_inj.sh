cd build
rm -rf *
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ../InjectFuncCall
make
cd ../

$LLVM_CLANG -O0 -emit-llvm -c ./inputs/input_for_hello.c -o ./inputs/input_for_hello.bc
# Run the pass through opt
$LLVM_OPT -load-pass-plugin ./build/libInjectFuncCall.so --passes="inject-func-call" ./inputs/input_for_hello.bc -o instrumented.bin
$LLVM_DIR/bin/lli instrumented.bin