# Ports Guide

This guide covers port build flags, CGO configuration, Node N-API, and Rust binding notes.

> Back to root: [`../../AGENTS.md`](../../AGENTS.md)

## Port Build Flags

Ports are enabled with CMake flags. Default ON: `OPTION_BUILD_PORTS_NODE`, `OPTION_BUILD_PORTS_PY`, `OPTION_BUILD_PORTS_RB`.

| CMake Option | Language |
|---|---|
| `OPTION_BUILD_PORTS_CS` | C# |
| `OPTION_BUILD_PORTS_CXX` | C++ |
| `OPTION_BUILD_PORTS_D` | D |
| `OPTION_BUILD_PORTS_GO` | Go |
| `OPTION_BUILD_PORTS_JAVA` | Java |
| `OPTION_BUILD_PORTS_JS` | JavaScript |
| `OPTION_BUILD_PORTS_LUA` | Lua |
| `OPTION_BUILD_PORTS_NODE` | NodeJS (ON) |
| `OPTION_BUILD_PORTS_PHP` | PHP |
| `OPTION_BUILD_PORTS_PL` | Perl |
| `OPTION_BUILD_PORTS_PY` | Python (ON) |
| `OPTION_BUILD_PORTS_R` | R |
| `OPTION_BUILD_PORTS_RB` | Ruby (ON) |
| `OPTION_BUILD_PORTS_RS` | Rust |
| `OPTION_BUILD_PORTS_TS` | TypeScript |
| `OPTION_BUILD_PORTS_ZIG` | Zig |

### Build Example

```sh
cmake -DOPTION_BUILD_PORTS_GO=On -DOPTION_BUILD_PORTS_RS=On ..
cmake --build . --target install
```

## Go Port — CGO Configuration

When using precompiled MetaCall binaries (Linux), `go build` may need CGO environment variables:

```sh
export CGO_CFLAGS="-I/gnu/store/$(ls /gnu/store/ | grep metacall | head -n 1)/include"
export CGO_LDFLAGS="-L/gnu/store/$(ls /gnu/store/ | grep metacall | head -n 1)/lib"
```

At runtime, set `LD_LIBRARY_PATH`:

```sh
export LD_LIBRARY_PATH="/gnu/store/$(ls /gnu/store/ | grep metacall | head -n 1)/lib"
```

### Go Port Testing

```sh
go test
go test -bench=.
```

## Node Port — N-API Notes

The Node port uses Node N-API for native addon communication. Key considerations:

- N-API is versioned and stable across NodeJS releases (since NodeJS 10.x).
- Thread-safe functions (`napi_threadsafe_function`) are used for cross-thread event loop submission.
- The port depends on `node_loader` being built and available.
- Use `metacall` npm package for the high-level API.

### Node Port Testing

```sh
node test.js
```

## Rust Port — Binding Notes

The Rust port wraps the MetaCall C API via `metacall-sys` FFI bindings.

### Linking with `metacall-sys`

Use `metacall-sys` as a build dependency to locate MetaCall:

`Cargo.toml`:
```toml
[build-dependencies]
metacall-sys = "0.1.2"
```

`build.rs`:
```rust
fn main() {
    metacall_sys::build();
}
```

### Rust Port Testing

```sh
cargo test
```

### Rust Toolchain

Check `source/ports/rs_port/rust-toolchain` for the pinned Rust version. The port requires Rust 1.55.0+.

## Port Limitations

Ports depend on loaders and inherit the same threading and fork safety constraints. See [`source/loaders/AGENTS.md`](../loaders/AGENTS.md) for details.

- Different maturity levels between ports.
- Host language specific deadlocks when multiple runtimes are mixed.
- Duplication of conversion logic between ports.
