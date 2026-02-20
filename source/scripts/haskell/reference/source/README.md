# Haskell Reference Bridge

This directory contains a minimal Haskell shared library that exports the ABI expected by `hs_loader`.

## Exports

- `metacall_hs_module_initialize`
- `metacall_hs_module_destroy`
- `metacall_hs_module_function_count`
- `metacall_hs_module_function_name`
- `metacall_hs_module_function_args_count`
- `metacall_hs_module_function_arg_name`
- `metacall_hs_module_function_arg_type`
- `metacall_hs_module_function_return_type`
- `metacall_hs_module_invoke`
- `metacall_hs_module_value_destroy`
- `metacall_hs_module_last_error`

It exposes two functions to MetaCall:

- `hs_answer() -> Int` (returns `42`)
- `hs_add(left: Int, right: Int) -> Int`

## Build

### Linux / macOS

```sh
./ghc-build.sh
```

### Windows (PowerShell)

```powershell
.\ghc-build.ps1
```

Both commands generate a dynamic library in this folder (`metacall_hs_bridge.so`, `.dylib`, or `.dll`).
On Windows, the build scripts also export the `metacall_hs_module_*` ABI symbols expected by `hs_loader`.

## Load

Use the produced shared library path with loader tag `hs`:

```c
metacall_load_from_file("hs", paths, size, NULL);
```
