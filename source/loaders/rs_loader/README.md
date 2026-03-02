# Rust Loader

### What is Rust Loader 

The Rust loader (`rs_loader`) is integrated into MetaCall's top level CMake build system. 
Running `cargo build` directly inside the Rust directory may appear to work, but it can cause toolchain, linker, and metadata mismatches with MetaCall.
Using the CMake build flow ensures the correct Rust nightly toolchain, linker paths, and test configuration are set up automatically.

### How the Rust Loader Works

MetaCall does not execute Rust source code directly.
Instead, the Rust loader compiles the crate into a shared library and then dynamically loads it at runtime using FFI.

The process:

1. MetaCall detects a Rust script (.rs)
2. The loader generates a temporary Cargo project
3. The crate is compiled into a shared library (.so)
4. Exported functions are discovered through metadata
5. Functions become callable from MetaCall CLI/Library.

### Why Nightly Rust is Required

The Rust loader depends on unstable compiler features and `rustc` compiler API, used for metadata extraction and symbol inspection.
These APIs are only available in the Rust nightly toolchain.
Using stable Rust typically results in the loader compiling but exported functions not appearing in inspect.

### Building 

Use CMake for building `rs_loader`, check the (README)[/docs/README.md] for more info.

### Tested Environment

- OS: Ubuntu (WSL2 on Windows)
- Build system: CMake
- Rust toolchain: nightly  

**The loader may fail to compile or load functions if stable Rust is used instead of nightly.**

### Building MetaCall with Rust Loader Enabled

```sh
mkdir build
cd build
cmake .. -DOPTION_BUILD_LOADERS_RS=ON -DOPTION_BUILD_CLI=ON
make -j4
```

### Verify the Build

After a successful build you should see:

```sh
Built target metacallcli
```

Locate the CLI:

```sh
find . -name metacallcli
```

Run it:

```sh
./metacallcli
```

This verifies the CLI started. Next we must confirm the Rust Loader works.

### Load a Rust Script

Example script is already included:
source/scripts/rust/basic/source/basic.rs

From the build directory run:

```sh
./metacallcli
```

Inside the CLI:

```sh
load rs ../source/scripts/rust/basic/source/basic.rs
```

List exported functions:

```sh
inspect
```

You should see functions like:

```sh
add(num_1, num_2)
add_float(num_1, num_2)
return_map()
```

### Call a Rust Function

Important: functions must be called using "call".

```sh
call add(1,2)
```
Output:
3

**The Rust loader is now successfully integrated with MetaCall and exported Rust functions can be invoked from the CLI.**

### Supported Types

|   Rust Type       |   MetaCall    |
|-------------------|---------------|
|   i32             |   number      |
|   f32             |   number      |
|   &str            |   string      |
|   String          |   string      |
|   Vec<T           |   array       |
|   Hashmaps<K,V>   |   map         |
