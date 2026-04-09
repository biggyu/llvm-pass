cmake -S . -B build -DLLVM_DIR=$(llvm-config --cmakedir) -DENABLE_RUNTIME_TIME=OFF
cmake --build build