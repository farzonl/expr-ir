mkdir build; cd build
# set LLVM_DIR to llvm-build-dir/lib/cmake/llvm/
cmake -GNinja -DLLVM_DIR=$LLVM_DIR ../; cd -
ninja -C ./build 

