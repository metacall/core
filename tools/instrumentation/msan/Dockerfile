FROM debian:trixie

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git ca-certificates \
    python3 python3-dev \
    clang lld libclang-rt-dev llvm-dev

RUN git clone --depth=1 https://github.com/llvm/llvm-project -b "llvmorg-19.1.6" /msan/llvm-project

# Stage 1: build clang without MSan
RUN cmake -G Ninja -B /msan/clang_build/ \
    -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_TARGETS_TO_BUILD=Native \
    -S /msan/llvm-project/llvm && \
    ninja -C /msan/clang_build/ -j$(nproc)

# Stage 2: build libc++ WITH MSan using stage 1 clang
RUN cmake -G Ninja -B /msan/cxx_build/ \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins \
    -DCMAKE_C_COMPILER=/msan/clang_build/bin/clang \
    -DCMAKE_CXX_COMPILER=/msan/clang_build/bin/clang++ \
    -DLLVM_TARGETS_TO_BUILD=Native \
    -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF \
    -DLIBCXXABI_USE_LLVM_UNWINDER=OFF \
    -DCMAKE_INSTALL_PREFIX=/opt/llvm-msan \
    -S /msan/llvm-project/runtimes && \
    ninja -C /msan/cxx_build/ -j$(nproc) && \
    ninja -C /msan/cxx_build/ install

WORKDIR /build/gtest

RUN printf "[memory]\nsrc:*/googletest/*\nfun:testing::*\n" > /build/gtest/msan-ignorelist.txt && \
    git clone --depth=1 https://github.com/google/googletest.git && \
    cmake -S googletest -B gtest-msan-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/gtest-msan \
        -DCMAKE_C_COMPILER=/msan/clang_build/bin/clang \
        -DCMAKE_CXX_COMPILER=/msan/clang_build/bin/clang++ \
        -DCMAKE_CXX_FLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -stdlib=libc++ -isystem /opt/llvm-msan/include/c++/v1 -fsanitize-ignorelist=/build/gtest/msan-ignorelist.txt" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/llvm-msan/lib -Wl,-rpath,/opt/llvm-msan/lib" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/llvm-msan/lib -Wl,-rpath,/opt/llvm-msan/lib" && \
    cmake --build gtest-msan-build -j$(nproc) && \
    cmake --install gtest-msan-build

RUN rm -rf /msan/llvm-project /msan/cxx_build

ENV PATH="/msan/clang_build/bin:/opt/gtest-msan/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/llvm-msan/lib:/opt/gtest-msan/lib:$LD_LIBRARY_PATH"
CMD ["bash"]