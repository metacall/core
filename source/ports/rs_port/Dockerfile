# docker build -t metacall/rs-port .
# docker run --rm -it metacall/rs-port

FROM metacall/core:dev AS develop

# Install dependencies
RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		clang-11 clang-format-11 libclang-11-dev libtcc-dev valgrind libdw-dev libbfd-dev libdwarf-dev libffi-dev \
	&& curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y \
	&& . "/root/.cargo/env" \
	&& rustup component add rustfmt \
	&& rustup toolchain add nightly \
	&& rustup component add clippy

ENV PATH="${PATH}:/root/.cargo/bin"

RUN cd build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Debug \
		-DOPTION_BUILD_DETOURS=Off \
		-DOPTION_BUILD_EXAMPLES=Off \
		-DOPTION_BUILD_LOADERS_C=On \
		-DOPTION_BUILD_LOADERS_NODE=On \
		-DOPTION_BUILD_LOADERS_PY=On \
		-DOPTION_BUILD_LOADERS_TS=On \
		-DOPTION_BUILD_SCRIPTS=Off \
		-DOPTION_BUILD_SERIALS_RAPID_JSON=On \
		-DOPTION_BUILD_TESTS=Off \
		.. \
	&& cmake --build . --target install \
	&& cd /usr/local/lib \
	&& ldconfig

WORKDIR /usr/local/metacall/source/ports/rs_port

COPY . .

CMD ["cargo", "test"]
