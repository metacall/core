# Loaders Guide

This guide covers loader build options, the threading model, and fork safety notes.

> Back to root: [`../../AGENTS.md`](../../AGENTS.md)

## Loader Build Matrix

Enable loaders with CMake flags. All loaders are OFF by default except `ext_loader` and `mock_loader`.

| CMake Option | Runtime | Version |
|---|---|---|
| `OPTION_BUILD_LOADERS_PY` | Python | 3.9 C API |
| `OPTION_BUILD_LOADERS_NODE` | NodeJS | v12.21.0 |
| `OPTION_BUILD_LOADERS_RB` | Ruby | 2.7 C API |
| `OPTION_BUILD_LOADERS_CS` | C# CoreCLR | 5.0.17 |
| `OPTION_BUILD_LOADERS_TS` | TypeScript | 3.9.7 |
| `OPTION_BUILD_LOADERS_JAVA` | Java | JVM |
| `OPTION_BUILD_LOADERS_WASM` | WebAssembly | VM |
| `OPTION_BUILD_LOADERS_C` | C | FFI |
| `OPTION_BUILD_LOADERS_RS` | Rust | 1.55.0 |
| `OPTION_BUILD_LOADERS_JL` | Julia | 1.6 |
| `OPTION_BUILD_LOADERS_LUA` | LuaJIT2 | v2.1 (OpenResty) |
| `OPTION_BUILD_LOADERS_DART` | Dart | VM 2.8.4 |
| `OPTION_BUILD_LOADERS_CR` | Crystal | 0.33.0 |
| `OPTION_BUILD_LOADERS_COB` | GNU/COBOL | 2.2 |
| `OPTION_BUILD_LOADERS_JSM` | SpiderMonkey | 4.8 |
| `OPTION_BUILD_LOADERS_LLVM` | LLVM | 11 |
| `OPTION_BUILD_LOADERS_RPC` | cURL RPC | - |
| `OPTION_BUILD_LOADERS_FILE` | File System | - |
| `OPTION_BUILD_LOADERS_EXT` | Core Extensions | ON by default |
| `OPTION_BUILD_LOADERS_MOCK` | Mock (testing) | ON by default |

### Build Example

```sh
cmake -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_NODE=On ..
cmake --build . --target install
```

### Test a Single Loader

```sh
ctest -VV -R metacall-python-test
```

## Threading Model

The threading model is experimental and may change.

### NodeJS Threading

- The V8 instance runs in a dedicated thread; the event loop blocks that thread.
- All calls are submitted to the event loop via N-API thread-safe mechanisms.
- The calling thread waits on a condition variable until the call completes.
- Recursive synchronous calls are detected via an atomic + thread-id check to avoid deadlock.

### Python Threading

- Python uses the Global Interpreter Lock (GIL) for thread safety.
- The Python event loop can be decoupled from the interpreter thread (work in progress).
- Python can run multiple interpreter instances in newer versions (not yet implemented).

### Cross-Language Threading

- **NodeJS host + other loaders**: Node Loader is reentrant. Python GIL is safe. Recursive calls always execute in V8 thread. Destroy order matters — loaders must be destroyed in initialization order within the correct thread.
- **Python host + NodeJS**: Python GIL and Node submission queue interact. Be cautious of callback resolution across threads.

### Loader Destruction Order

Each loader registers its initialization order. During destruction, children are destroyed recursively. Use:

```c
loader_initialization_register(impl);
```

on init, and iterate the tree on destruction.

## Fork Safety

MetaCall implements a fork-safe model using **detours** (not `pthread_atfork`).

### How It Works

1. Intercept the fork call (`fork` on POSIX, `RtlCloneUserProcess` on Windows).
2. Shutdown all runtimes by unloading all loaders.
3. Execute the real fork function.
4. Restore all runtimes by reloading all loaders.
5. Execute user-defined fork callback if registered.

### Key Points

- Fork **safety** is preserved; runtime **state** is not.
- NodeJS thread pool is dead in the child after fork (fork-one model on POSIX).
- Use `clone` instead of `fork` to bypass interception (not recommended).
- Register callbacks via `metacall_fork()` to re-load scripts after fork.

### Why Detours Instead of `pthread_atfork`

- `pthread_atfork` is POSIX-only (not cross-platform).
- `pthread_atfork` has a design bug that cannot restore NodeJS mutexes.
- `pthread_atfork` may be formally deprecated in future POSIX versions.

### Caveats

- Detours modify process memory at runtime — platform dependent, not 100% safe.
- If MetaCall is embedded, register fork callbacks to restore state.
