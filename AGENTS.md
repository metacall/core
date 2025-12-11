# AGENTS.md

This file provides guidance to LLMs when working with code in this repository.

## Project Overview

MetaCall is a polyglot runtime that enables calling functions, methods, and procedures between multiple programming languages. It supports Python, NodeJS, TypeScript, Ruby, C#, Java, WASM, Go, C, C++, Rust, and more through a plugin-based architecture.

## Build Commands

### Basic Build
```sh
mkdir build && cd build
cmake ..
cmake --build . --target install
```

### Build with Specific Loaders
```sh
cmake -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_NODE=On -DOPTION_BUILD_LOADERS_RB=On ..
```

### Common Build Options
- `OPTION_BUILD_LOADERS_PY` - Python loader
- `OPTION_BUILD_LOADERS_NODE` - NodeJS loader
- `OPTION_BUILD_LOADERS_RB` - Ruby loader
- `OPTION_BUILD_LOADERS_CS` - C# loader
- `OPTION_BUILD_LOADERS_TS` - TypeScript loader
- `OPTION_BUILD_LOADERS_JAVA` - Java loader
- `OPTION_BUILD_LOADERS_WASM` - WebAssembly loader
- `OPTION_BUILD_LOADERS_C` - C loader
- `OPTION_BUILD_LOADERS_RS` - Rust loader
- `OPTION_BUILD_TESTS` - Build tests (default ON)
- `OPTION_BUILD_EXAMPLES` - Build examples (default ON)
- `CMAKE_BUILD_TYPE` - Debug/Release/RelWithDebInfo/MinSizeRel

### Docker Development
```sh
./docker-compose.sh build   # Build all Docker images
./docker-compose.sh test    # Run tests in Docker
```

## Testing

### Run All Tests
```sh
cd build
ctest
```

### Run a Single Test
```sh
ctest -VV -R metacall-python-test
```

### Run Tests with Regex Pattern
```sh
ctest -R "metacall-node.*"
```

### Build and Run a Specific Test
```sh
# Build required dependencies and test
make py_loader metacall-python-test
ctest -VV -R metacall-python-test
```

### Run Tests with Valgrind
```sh
cmake -DOPTION_TEST_MEMORYCHECK=On ..
make memcheck
```

### Run Tests with Sanitizers
```sh
# Address Sanitizer
cmake -DOPTION_BUILD_ADDRESS_SANITIZER=On ..

# Thread Sanitizer
cmake -DOPTION_BUILD_THREAD_SANITIZER=On ..
```

## Code Formatting

Format C/C++ code using clang-format:
```sh
cmake --build build --target clang-format
```

## Architecture

### Core Modules (source/)

- **metacall/** - Main library providing the public C API (`metacall.h`)
- **reflect/** - Type system, values, and function abstractions for cross-language interop
- **loader/** - Plugin interface for loading language runtimes
- **loaders/** - Runtime implementations (py_loader, node_loader, rb_loader, etc.)
- **serial/** - Serialization plugin interface
- **serials/** - Serialization implementations (rapid_json_serial)
- **detour/** - Function hooking interface for patching C functions at runtime
- **detours/** - Detour implementations (plthook_detour)
- **ports/** - Language bindings to use MetaCall from other languages
- **adt/** - Abstract data types (vector, set, hashmap)
- **dynlink/** - Cross-platform dynamic library loading
- **cli/** - Command-line interface (metacallcli)

### Plugin System

MetaCall uses a plugin architecture at multiple levels:
1. **Loaders** - Embed language runtimes (each loader implements `loader_impl_interface`)
2. **Serials** - Handle (de)serialization of values
3. **Detours** - Patching C functions to work within existing runtimes (e.g., node.exe, python.exe)

### Type System

The reflect module provides an abstract type system with these supported types:
- Boolean, Char, Short, Int, Long, Float, Double
- String, Buffer, Array, Map
- Pointer, Null, Future, Function
- Class, Object

### Key Design Patterns

- Loaders must implement: `initialize`, `execution_path`, `load_from_file`, `load_from_memory`, `load_from_package`, `clear`, `discover`, `destroy`
- Values use an object pool with memory layout: [DATA][TYPE_ID]
- Fork safety is achieved through detours that intercept fork calls and reinitialize runtimes

### Environment Variables

- `LOADER_LIBRARY_PATH` - Directory for loader plugins
- `LOADER_SCRIPT_PATH` - Directory for scripts to load
- `SERIAL_LIBRARY_PATH` - Directory for serial plugins
- `DETOUR_LIBRARY_PATH` - Directory for detour plugins
- `CONFIGURATION_PATH` - Path to global.json configuration

### Test Structure

Tests are in `source/tests/` with naming convention `metacall_<loader>_test` or `metacall_<feature>_test`. Each test links against GTest and the metacall library.

## Important Notes

- `metacall_initialize` and `metacall_destroy` must be called from the same thread
- Tests require appropriate loaders to be built (check CMakeLists.txt conditions)
