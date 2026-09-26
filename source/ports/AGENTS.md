# Ports Guide

> Back to root: [`../../AGENTS.md`](../../AGENTS.md)
> Loader threading and fork limits apply here: [`../loaders/AGENTS.md`](../loaders/AGENTS.md)

This guide covers port build flags, Go CGO, Node test entry, and Rust linking.

## Build Gate

Ports build only when the master gate is `ON`:

```sh
cmake -DOPTION_BUILD_PORTS=On -S . -B build
```

The gate defaults to `OFF` (`CMakeLists.txt`). Each port flag defaults to `OFF`, except `NODE`, `PY`, `RB` which default to `ON`.

## Port Matrix

Flags match `source/ports/CMakeLists.txt` exactly.

| CMake Option | Language | Default |
|---|---|---|
| `OPTION_BUILD_PORTS_CS` | C# | `OFF` |
| `OPTION_BUILD_PORTS_CXX` | C++ | `OFF` |
| `OPTION_BUILD_PORTS_D` | D | `OFF` |
| `OPTION_BUILD_PORTS_GO` | Go | `OFF` |
| `OPTION_BUILD_PORTS_JAVA` | Java | `OFF` |
| `OPTION_BUILD_PORTS_JS` | JavaScript, V8 plus SWIG | `OFF` |
| `OPTION_BUILD_PORTS_LUA` | Lua | `OFF` |
| `OPTION_BUILD_PORTS_NODE` | NodeJS | `ON` |
| `OPTION_BUILD_PORTS_PHP` | PHP | `OFF` |
| `OPTION_BUILD_PORTS_PL` | Perl | `OFF` |
| `OPTION_BUILD_PORTS_PY` | Python | `ON` |
| `OPTION_BUILD_PORTS_R` | R | `OFF` |
| `OPTION_BUILD_PORTS_RB` | Ruby | `ON` |
| `OPTION_BUILD_PORTS_RS` | Rust | `OFF` |
| `OPTION_BUILD_PORTS_TS` | TypeScript | `OFF` |
| `OPTION_BUILD_PORTS_ZIG` | Zig | `OFF` |

Unwired orphans exist without CMake entries: `source/ports/nim_port/` and `source/ports/scala_port/`. Do not claim them as supported.

### Build Example

```sh
cmake -DOPTION_BUILD_PORTS=On -DOPTION_BUILD_PORTS_GO=On -DOPTION_BUILD_PORTS_RS=On -S . -B build
cmake --build build --target install
```

## Go Port: CGO

`source/ports/go_port/CMakeLists.txt` passes `CGO_CFLAGS` with MetaCall include dirs and `CGO_LDFLAGS` with the built lib. With precompiled binaries on Linux, `go build` may need manual vars (`source/ports/go_port/source/README.md`):

```sh
export CGO_CFLAGS="-I/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/include"
export CGO_LDFLAGS="-L/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/lib"
```

At runtime set the lib path:

```sh
export LD_LIBRARY_PATH="/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/lib"
```

### Go Port Testing

From `source/ports/go_port/source/` (the `go.mod` location):

```sh
go test
go test -bench=.
```

## Node Port: Test Entry

Package `metacall` (`source/ports/node_port/package.json`). `test.js` builds a Mocha runner over `source/ports/node_port/test/`. From `source/ports/node_port/`:

```sh
node test.js
```

## Rust Port: Linking

Toolchain file `source/ports/rs_port/rust-toolchain` pins `stable`. For out-of-tree projects, use `metacall-sys` as a build dependency (`source/ports/rs_port/README.md`):

```toml
[build-dependencies]
metacall-sys = "0.1.2"
```

```rust
fn main() {
    metacall_sys::build();
}
```

### Rust Port Testing

From `source/ports/rs_port/`:

```sh
cargo test
```

The integration tests in `rs_port/tests/` need a built or installed MetaCall library. They fail without it.

## Port Limits

- Ports depend on loaders. They inherit loader threading and fork limits.
- Maturity differs per port. Mixed runtimes can deadlock across threads.
- `js_port` needs V8 plus SWIG per `source/ports/js_port/CMakeLists.txt`. It is separate from the deprecated V8 `js_loader`.
