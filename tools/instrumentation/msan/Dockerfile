FROM debian:trixie

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build git \
    clang libclang-rt-dev llvm \
    python3-dev

# Build LLVM/libc++ with MSan instrumentation


WORKDIR /build/llvm

RUN git clone --depth=1 https://github.com/llvm/llvm-project.git llvm-project && \
    cmake -S llvm-project/llvm -B llvm-msan-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS="clang" \
        -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
        -DLLVM_USE_SANITIZER=MemoryWithOrigins \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_INSTALL_PREFIX=/opt/llvm-msan && \
    cmake --build llvm-msan-build --target cxx cxxabi clang && \
    cmake --install llvm-msan-build

# Build GoogleTest with MSan instrumentation
WORKDIR /build/gtest

RUN git clone --depth=1 https://github.com/google/googletest.git && \
    cmake -S googletest -B gtest-msan-build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/llvm-msan \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_CXX_FLAGS="-fsanitize=memory -fsanitize-memory-track-origins=2 -stdlib=libc++ -fno-omit-frame-pointer" \
        -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++" \
        -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=memory -stdlib=libc++" && \
    cmake --build gtest-msan-build && \
    cmake --install gtest-msan-build

ENV PATH="/opt/llvm-msan/bin:$PATH"
ENV LD_LIBRARY_PATH="/opt/llvm-msan/lib:$LD_LIBRARY_PATH"

CMD ["bash"]
