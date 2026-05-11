FROM debian:trixie-slim

RUN echo "deb-src http://deb.debian.org/debian trixie main" >> /etc/apt/sources.list \
	&& apt-get update \
	&& apt-get install -y --no-install-recommends \
		build-essential \
		valgrind \
		dpkg-dev \
		git \
		curl \
		ca-certificates \
	&& curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | sh -s -- --debug

WORKDIR /build

RUN apt-get source python3.13 \
	&& apt-get build-dep -y python3.13 \
	&& cd python3.13-* \
	&& sed -i 's|\/\* #define Py_USING_MEMORY_DEBUGGER \*\/|#define Py_USING_MEMORY_DEBUGGER|' Objects/obmalloc.c \
	&& ./configure \
		--with-pydebug \
		--without-pymalloc \
		--with-valgrind \
		--with-ensurepip=no \
	&& make -j$(nproc) \
	&& make altinstall

COPY source/ports/py_port /app/py_port
COPY source/scripts /app/scripts
COPY source/tests/memcheck /app/memcheck

WORKDIR /app/py_port

RUN mkdir -p scripts \
	&& touch scripts/sometestmock.mock \
	&& cp /app/scripts/python/helloworld/source/helloworld.py scripts/ \
	&& cp /app/scripts/python/example/source/example.py scripts/ \
	&& cp /app/scripts/ruby/hello/source/hello.rb scripts/ \
	&& cp /app/scripts/ruby/second/source/second.rb scripts/ \
	&& cp /app/scripts/node/derpyramda/source/derpyramda.js scripts/

ENV LOADER_SCRIPT_PATH=/app/py_port/scripts

RUN valgrind \
	--tool=memcheck \
	--leak-check=full \
	--show-leak-kinds=all \
	--num-callers=500 \
	--trace-children=yes \
	--track-origins=yes \
	--error-limit=no \
	--max-stackframe=8368224 \
	--read-var-info=yes \
	--show-reachable=yes \
	--error-exitcode=1 \
	--suppressions=/app/memcheck/valgrind-dl.supp \
    --suppressions=/app/memcheck/valgrind-python.supp \
	--suppressions=/app/memcheck/valgrind-python-all.supp \
	python3.13 test.py
