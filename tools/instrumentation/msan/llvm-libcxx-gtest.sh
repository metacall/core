#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./build-llvm-libcxx-gtest.sh <llvm-project-src> <build-dir> <install-dir>
# Example:
#   ./build-llvm-libcxx-gtest.sh ~/src/llvm-project ~/build/llvm-msan ~/opt/llvm-msan

if [[ $# -ne 3 ]]; then
  echo "Usage: $0 <llvm-project-src> <build-dir> <install-dir>"
  exit 1
fi

LLVM_SRC="$1"
BUILD_DIR="$2"
INSTALL_DIR="$3"

cmake -S "$LLVM_SRC/llvm" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
  -DLLVM_USE_SANITIZER=MemoryWithOrigins \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++

cmake --build "$BUILD_DIR" --target cxx cxxabi clang
cmake --install "$BUILD_DIR"

echo "LLVM/libc++ instrumentation build completed at: $INSTALL_DIR"

# Build GoogleTest with MSan instrumentation
GTEST_SRC_DIR="/tmp/googletest"
GTEST_BUILD_DIR="/tmp/googletest-build"

if [ ! -d "$GTEST_SRC_DIR" ]; then
  git clone https://github.com/google/googletest.git "$GTEST_SRC_DIR"
fi

cmake -S "$GTEST_SRC_DIR" -B "$GTEST_BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2 -stdlib=libc++ -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++"

cmake --build "$GTEST_BUILD_DIR"
cmake --install "$GTEST_BUILD_DIR"

echo "GoogleTest with MSan instrumentation completed at: $INSTALL_DIR"

echo "Note: gtest should be configured with clang and MSan flags plus -stdlib=libc++."
