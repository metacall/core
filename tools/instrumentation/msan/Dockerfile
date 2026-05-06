FROM debian:trixie

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git ca-certificates \
    python3 python3-dev \
    clang lld libclang-rt-dev libclang-19-dev llvm-19-dev

WORKDIR /build

# Fetch LLVM monorepo once (for runtimes source only)
RUN git clone --depth=1 https://github.com/llvm/llvm-project.git llvm-project

# Build runtimes with MSan using system clang
RUN cmake -S llvm-project/runtimes -B llvm-msan-runtimes -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=/usr/bin/clang \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
        -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
        -DLLVM_USE_SANITIZER=MemoryWithOrigins \
        -DLIBCXXABI_USE_LLVM_UNWINDER=OFF \
        -DLIBCXX_INCLUDE_BENCHMARKS=OFF \
        -DLIBCXX_ENABLE_CLANG_TIDY=OFF \
        -DCMAKE_INSTALL_PREFIX=/opt/llvm-msan && \
    cmake --build llvm-msan-runtimes && \
    cmake --install llvm-msan-runtimes

# Build GoogleTest with MSan using system clang and MSan runtimes
WORKDIR /build/gtest

RUN printf "[memory]\nsrc:*/googletest/*\nfun:testing::*\n" > /build/gtest/msan-ignorelist.txt && \
    git clone --depth=1 https://github.com/google/googletest.git && \
    cmake -S googletest -B gtest-msan-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/gtest-msan \
        -DCMAKE_C_COMPILER=/usr/bin/clang \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
        -DCMAKE_CXX_FLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -stdlib=libc++ -isystem /opt/llvm-msan/include/c++/v1 -fsanitize-ignorelist=/build/gtest/msan-ignorelist.txt" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/llvm-msan/lib -Wl,-rpath,/opt/llvm-msan/lib" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++ -L/opt/llvm-msan/lib -Wl,-rpath,/opt/llvm-msan/lib" && \
    cmake --build gtest-msan-build && \
    cmake --install gtest-msan-build

ENV PATH="/usr/bin:/opt/gtest-msan/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/llvm-msan/lib:/opt/gtest-msan/lib:$LD_LIBRARY_PATH"
CMD ["bash"]
