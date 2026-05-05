FROM debian:trixie

RUN echo "deb-src http://deb.debian.org/debian trixie main" >> /etc/apt/sources.list && \
    apt-get update && apt-get install -y \
    build-essential gdb valgrind dpkg-dev git curl

WORKDIR /build

RUN apt-get source python3.13 && \
    apt-get build-dep -y python3.13 && \
    cd python3.13-* && \
    sed -i 's|\/\* #define Py_USING_MEMORY_DEBUGGER \*\/|#define Py_USING_MEMORY_DEBUGGER|' Objects/obmalloc.c && \
    ./configure \
        --with-pydebug \
        --without-pymalloc \
        --with-valgrind \
        --with-ensurepip=no && \
    make -j$(nproc) && \
    make altinstall

RUN curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | sh -s -- --debug

COPY source/ports/py_port /app/py_port
COPY source/scripts /app/scripts
COPY source/tests/memcheck /app/memcheck

WORKDIR /app/py_port

RUN mkdir -p scripts && \
    touch scripts/sometestmock.mock && \
    cp /app/scripts/python/helloworld/source/helloworld.py scripts/ && \
    cp /app/scripts/python/example/source/example.py scripts/ && \
    cp /app/scripts/ruby/hello/source/hello.rb scripts/ && \
    cp /app/scripts/ruby/second/source/second.rb scripts/ && \
    cp /app/scripts/node/derpyramda/source/derpyramda.js scripts/

ENV LOADER_SCRIPT_PATH=/app/py_port/scripts

CMD ["valgrind", "--leak-check=full", "--num-callers=30", \
    "--suppressions=/app/memcheck/valgrind-dl.supp", \
    "--suppressions=/app/memcheck/valgrind-python.supp", \
    "--suppressions=/app/memcheck/valgrind-node.supp", \
    "--suppressions=/app/memcheck/valgrind-wasm.supp", \
    "--suppressions=/app/memcheck/valgrind-ruby.supp", \
    "python3.13", "test.py"]