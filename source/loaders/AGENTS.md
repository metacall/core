# Loaders Guide

> Back to root: [`../../AGENTS.md`](../../AGENTS.md)

This guide covers loader build flags, config generation, threading, and fork safety.

## Build Gate

Loaders build only when the master gate is `ON`:

```sh
cmake -DOPTION_BUILD_LOADERS=On -S . -B build
```

The gate defaults to `ON` (`CMakeLists.txt`). Each loader flag defaults to `OFF`, except `EXT` and `MOCK` which default to `ON`.

## Loader Matrix

Flags and versions match `source/loaders/CMakeLists.txt` option descriptions exactly. `js_loader` (V8 5.1+) still builds through `OPTION_BUILD_LOADERS_JS`, default `OFF`. It is legacy; use `node_loader` or `ts_loader` for JavaScript.

| CMake Option | Runtime | Notes |
|---|---|---|
| `OPTION_BUILD_LOADERS_C` | C FFI | libffi plus libclang plus libtcc |
| `OPTION_BUILD_LOADERS_COB` | GNU/COBOL 2.2 | - |
| `OPTION_BUILD_LOADERS_CR` | Crystal 0.33.0 | - |
| `OPTION_BUILD_LOADERS_CS` | C# CoreCLR 5.0.17 | - |
| `OPTION_BUILD_LOADERS_DART` | Dart VM 2.8.4 | - |
| `OPTION_BUILD_LOADERS_EXT` | Core extensions | `ON` by default |
| `OPTION_BUILD_LOADERS_FILE` | File system | - |
| `OPTION_BUILD_LOADERS_JAVA` | JVM | - |
| `OPTION_BUILD_LOADERS_JS` | V8 5.1 | Legacy. Prefer `node_loader` or `ts_loader`. |
| `OPTION_BUILD_LOADERS_JL` | Julia 1.6 | - |
| `OPTION_BUILD_LOADERS_JSM` | SpiderMonkey 4.8 | Kept. Not the deprecated V8 loader. |
| `OPTION_BUILD_LOADERS_LLVM` | LLVM 11 | - |
| `OPTION_BUILD_LOADERS_LUA` | LuaJIT2 v2.1 OpenResty fork | - |
| `OPTION_BUILD_LOADERS_MOCK` | Mock, no deps | `ON` by default, core testing |
| `OPTION_BUILD_LOADERS_NODE` | NodeJS v12.21.0 | Flag is a floor, not a cap |
| `OPTION_BUILD_LOADERS_PY` | Python 3.9 C API | Flag is a floor, tested up to 3.13+ |
| `OPTION_BUILD_LOADERS_RB` | Ruby 2.7 C API | Flag is a floor |
| `OPTION_BUILD_LOADERS_RS` | Rust 1.55.0 | Flag is a floor |
| `OPTION_BUILD_LOADERS_RPC` | cURL RPC | - |
| `OPTION_BUILD_LOADERS_TS` | TypeScript 3.9.7 | Runs on Node runtime |
| `OPTION_BUILD_LOADERS_WASM` | WASM VM | - |

Unwired orphans exist without CMake entries: `source/loaders/deno_loader/` and `source/loaders/kind_loader/`. Do not claim them as supported.

### Build Example

```sh
cmake -DOPTION_BUILD_LOADERS=On -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_NODE=On -S . -B build
cmake --build build --target install
```

### Test a Single Loader

Loader tests also need scripts: `OPTION_BUILD_SCRIPTS` and the matching language flag, for example `OPTION_BUILD_SCRIPTS_PY` (`source/tests/metacall_python_test/CMakeLists.txt`). Then:

```sh
cd build
ctest -VV -R metacall-python-test
```

## Loader Config Generation

Each loader with native deps ships a JSON config from `source/loaders/loader.json.in`. Macros live in `source/loaders/CMakeLists.txt`:

- `loader_configuration_begin(target)` selects target and template.
- `loader_configuration_paths(list)` sets host search paths.
- `loader_configuration_deps(lib paths...)` sets shared library deps.
- `loader_configuration_env(VAR=value ...)` sets init-time env vars.
- `loader_configuartion_end()` writes dev plus install configs. The spelling matches the code; do not rename it.

## Threading Model

The model is experimental and may change.

### NodeJS

- V8 runs on a dedicated thread. The event loop blocks that thread.
- Calls submit to the loop via N-API thread-safe handles. The caller waits on a condition variable.
- Recursive sync calls use an atomic plus thread-id check to avoid deadlock.
- Fork uses fork-one: only the caller thread survives. The Node thread pool does not survive. See `source/loaders/node_loader/source/node_loader_impl.cpp` TODO block.

### Python

- Python uses the GIL. Acquire it via `PyGILState_Ensure` before API use. See `source/loaders/py_loader/source/py_loader_threading.cpp`.
- Keep GIL sections short. Do not block the GIL thread on a Node call without a clear handoff.

### Cross-Language

- Node host plus other loaders: Node loader is reentrant. Respect destroy order. Mixed Python plus Node callbacks can deadlock across threads.
- Destroy loaders in reverse initialization order: children unload before hosts. Each loader registers with `loader_initialization_register` (`source/loader/include/loader/loader.h`, `source/loader/source/loader.c`). The initialization-order vector and a stack drive ordered teardown (`source/loader/source/loader.c`).

## Fork Safety

MetaCall uses detours, not `pthread_atfork`. `pthread_atfork` is POSIX only and cannot restore Node mutexes. See the `node_loader` TODO note.

Flow:

1. Intercept `fork` with a detour.
2. Run the pre-fork callback if set.
3. Destroy the whole runtime with `metacall_destroy`.
4. Run the real fork.
5. Reinitialize with `metacall_initialize` in parent and child.
6. Run the post-fork callback with the child pid if set.

Key points:

- Fork safety is kept. Runtime state is not kept. Reload scripts after fork. Known limit: the Node loader cannot reinitialize after fork (`node_loader_impl.cpp`, TODO 2.0). Fork safety stays experimental for Node.
- Register callbacks with `metacall_fork` (`source/metacall/include/metacall/metacall_fork.h`). Init with `metacall_fork_initialize`, release with `metacall_fork_destroy`.
- Gate with `OPTION_FORK_SAFE`, default `ON`. The build force-disables it with `OPTION_BUILD_THREAD_SANITIZER` and no `SANITIZER_LIBRARIES_PATH` (root `CMakeLists.txt`).
