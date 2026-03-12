# Creating a New Loader for MetaCall

This guide explains how to add support for a new programming language or runtime to MetaCall by implementing a new **loader**. It is based on the process used when adding the [LLVM loader](https://github.com/metacall/core/commit/af60ad595e61d52d537bc528039471a2773c4f90) and the existing loader architecture.

## 1. Loader interface

Every loader must implement the `loader_impl_interface` defined in the core:

```c
typedef struct loader_impl_interface_type
{
  loader_impl_interface_initialize   initialize;
  loader_impl_interface_execution_path execution_path;
  loader_impl_interface_load_from_file   load_from_file;
  loader_impl_interface_load_from_memory load_from_memory;
  loader_impl_interface_load_from_package load_from_package;
  loader_impl_interface_clear         clear;
  loader_impl_interface_discover     discover;
  loader_impl_interface_destroy      destroy;
} * loader_impl_interface;
```

- **initialize** – Start the runtime and register the loader with the core. Optionally read configuration (e.g. from `configuration`). Return 0 on success.
- **execution_path** – Register a path used to resolve scripts/libraries (e.g. for `load_from_file` or `load_from_package`). Store it in loader-specific state if needed.
- **load_from_file** – Load code from one or more file paths. Return a handle that represents the loaded unit.
- **load_from_memory** – Load code from a buffer. Return a handle.
- **load_from_package** – Load a precompiled library or package (e.g. a shared library and its header). Return a handle.
- **clear** – Unload a handle and release resources.
- **discover** – Inspect a handle and register its functions (and optionally types) into the given `context` via the reflect API.
- **destroy** – Shut down the runtime and free loader state. Called when the loader is unloaded.

Implement these in a C file (for the plugin entry and glue) and C++ or C (for the actual implementation), following the pattern of existing loaders (e.g. `py_loader`, `node_loader`, `c_loader`, `llvm_loader`).

## 2. Directory layout

Create a new directory under `source/loaders/` named `&lt;name&gt;_loader`, for example `mylang_loader`:

```
source/loaders/mylang_loader/
├── CMakeLists.txt
├── include/
│   └── mylang_loader/
│       ├── mylang_loader.h           # Public C API (if any)
│       └── mylang_loader_impl.h      # Implementation details
└── source/
    ├── mylang_loader.c               # Plugin entry, initialize/destroy, etc.
    └── mylang_loader_impl.cpp        # load_from_*, clear, discover, execution_path
```

- **mylang_loader.c** – Expose the `loader_impl_interface` and the single exported symbol the plugin system expects (see existing loaders). Implement or delegate `initialize`, `destroy`, and `execution_path` here if they are thin.
- **mylang_loader_impl.cpp** – Implement loading (file/memory/package), clear, and discover. Use the reflect API to register functions (and types) so MetaCall can call into the runtime.

## 3. CMake integration

- In **source/loaders/CMakeLists.txt**:
  - Add an option, e.g. `OPTION_BUILD_LOADERS_MYLANG`.
  - Add `add_subdirectory(mylang_loader)` (guarded by that option and `OPTION_BUILD_LOADERS`).

- In **mylang_loader/CMakeLists.txt**:
  - Guard the whole file with `if(NOT OPTION_BUILD_LOADERS OR NOT OPTION_BUILD_LOADERS_MYLANG) return() endif()`.
  - Use `find_package` or similar for your runtime’s dependencies.
  - Define a `MODULE` library (plugin) with your sources and link it to the MetaCall loader API, reflect, log, and any dependency libraries.
  - Follow the same pattern as other loaders for `target_include_directories`, `target_compile_definitions`, and install/export if applicable.

## 4. Scripts and tests (mandatory)

- **Scripts** – If your loader can run code from files, add examples under `source/scripts/` (e.g. `source/scripts/mylang/`) and wire them in the scripts CMake so they are built/copied to a path that `LOADER_SCRIPT_PATH` or execution paths can use.
- **Tests** – Add a test under `source/tests/`, e.g. `metacall_mylang_test`, that:
  - Calls `metacall_initialize()`.
  - Loads a script or package with your loader (`metacall_load_from_file` / `metacall_load_from_memory` / `metacall_load_from_package`).
  - Uses `metacall()` or `metacallv()` to call discovered functions and checks results.
  - Calls `metacall_destroy()`.

Register the test in `source/tests/CMakeLists.txt` and set `TESTS_ENVIRONMENT_VARIABLES` (e.g. `LOADER_LIBRARY_PATH`, `LOADER_SCRIPT_PATH`) so the loader can find its scripts and libraries.

## 5. Type system and discovery

- Use the **reflect** module to create types and values that represent your runtime’s data (integers, strings, arrays, etc.).
- In **discover**, for each callable (function or method) in the loaded handle:
  - Build the function signature (argument types and return type) using the reflect API.
  - Register the function with the core so that `metacall` / `metacallv` can resolve it and forward calls to your loader’s invoke implementation.

If your language has a type system (e.g. from headers or AST), map its types to MetaCall’s type IDs (e.g. `METACALL_INT`, `METACALL_STRING`, `METACALL_ARRAY`, etc.) so that serialization and cross-language calls work correctly.

## 6. Conventions and tips

- **Naming** – Use a short tag for the loader (e.g. `mylang`). This tag is what users pass to `metacall_load_from_file("mylang", ...)` and similar.
- **Errors** – Use the project’s `log` API and return non-zero or NULL where appropriate so the core and tests can detect failures.
- **Threading** – If the runtime is not thread-safe, document it and consider whether the loader should run calls on a single thread (e.g. via a queue), similar to the Node loader.
- **Fork safety** – If the process may fork, ensure the runtime is reinitialized or unloaded around fork (see the fork model in the main docs); the core will unload/reload loaders on fork when detours are enabled.
- **Existing loaders** – Use a simple loader (e.g. `mock_loader`) or a small one (e.g. `file_loader`, or the C loader for a compiler-style loader) as a template and copy the structure, then replace the runtime-specific parts.

## 7. References

- [Loader interface and plugin system](README.md#53-plugins) in the main documentation.
- [LLVM loader commit](https://github.com/metacall/core/commit/af60ad595e61d52d537bc528039471a2773c4f90) for a full example of adding a new loader (options, CMake, scripts, tests).
- [Reflect API](README.md#52-reflect) for types, values, and function registration.
- [CONTRIBUTING](../.github/CONTRIBUTING.md) for general contribution and PR process.
