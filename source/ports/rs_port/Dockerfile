FROM devraymondsh/ubuntu-docker-rust

# Install dependencies
RUN apt-get update \
  && apt-get install -y --no-install-recommends build-essential cmake ca-certificates git nodejs npm pkg-config clang-11 clang-format-11 libclang-11-dev cmake valgrind libdw-dev libbfd-dev libdwarf-dev libffi-dev

WORKDIR /root/metacall-polyglot
RUN git clone --branch v0.7.3 https://github.com/metacall/core
RUN mkdir core/build

WORKDIR /root/metacall-polyglot/core/build
RUN cmake \
  -DOPTION_BUILD_DETOURS=Off \
  -DOPTION_BUILD_EXAMPLES=Off \
  -DOPTION_BUILD_LOADERS_C=On \
  -DOPTION_BUILD_LOADERS_NODE=On \
  -DOPTION_BUILD_SANITIZER=ON \
  -DOPTION_BUILD_SCRIPTS=Off \
  -DOPTION_BUILD_SERIALS_RAPID_JSON=On \
  -DOPTION_BUILD_TESTS=Off \
  ..
RUN cmake --build . --target install
RUN cd /usr/local/lib && ldconfig

RUN rustup component add rustfmt
RUN rustup toolchain add nightly
RUN cargo install cargo-valgrind

WORKDIR /root/metacall-polyglot
CMD ["cargo", "test"]