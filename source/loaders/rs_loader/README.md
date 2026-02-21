# Rust Loader

## Building

The Rust loader (`rs_loader`) is integrated into MetaCall's top level CMake build system. 
While invoking `cargo build` directly inside the Rust directory may work in isolation, but it can lead to toolchain, linker and dependency mismatches.
Using the CMake build flow ensures the correct Rust nightly toolchain, linker paths, and test configuration are set up automatically.

```sh
git clone https://github.com/metacall/core.git
mkdir core/build
cd core/build
cmake -DOPTION_BUILD_LOADERS_RS=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
ctest -VV -R metacall-rs-test
```
