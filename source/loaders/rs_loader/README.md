# Rust Loader

## Building

This document explains how to successfully build and test the MetaCall Rust loader from a clean environment.

The Rust loader (`rs_loader`) is integrated into MetaCall's top level CMake build system. 
Running cargo `build` directly inside the Rust directory may appear to work, but it can cause toolchain, linker, and metadata mismatches with MetaCall.
Using the CMake build flow ensures the correct Rust nightly toolchain, linker paths, and test configuration are set up automatically.

### Tested Environment

- OS: Ubuntu (WSL2 on Windows)
- Build system: CMake
- Rust toolchain: nightly 
- NodeJS: v18+
- C/C++ compiler: gcc/g++ 

***The loader may fail to compile or load functions if stable Rust is used instead of nightly.***

### 1. Install System Dependencies

Install required build tools:

```sh
sudo apt update
sudo apt install build-essential cmake git python3 pkg-config libssl-dev nodejs npm
```

### 2. Install Rust (Nightly Required)

MetaCall Rust loader requires compiler APIs only available in nightly Rust.

```sh
curl https://sh.rustup.rs -sSf | sh
source $HOME/.cargo/env
rustup toolchain install nightly
rustup default nightly
```

```sh
rustc --version
```
It should show nightly.

most of MetaCall Rust failures happen due to:
nightly is installed but shell still uses stable.

This command shows the active toolchain.
```sh
rustup show
```
Expected:

```sh
Default host: x86_64-unknown-linux-gnu
active toolchain: nightly-xxxx
```

### 3. Clone MetaCall

Fork the repository first on GitHub, then:

```sh
git clone https://github.com/<your-username>/core.git metacall
cd metacall
```
Optional but recommended

```sh
git remote add upstream https://github.com/metacall/core.git
```
### 4. IMPORTANT: Do NOT Build Rust Loader Manually

Do NOT run:
`cargo build`

inside "rs_loader/rust".

MetaCall controls Rust compilation through CMake.
Running cargo manually may cause linker, metadata and loader mismatches.

### 5. Build MetaCall with Rust Loader

Create build directory:

```sh
mkdir build
cd build
```

Configure:

```sh
cmake .. \
-DOPTION_BUILD_LOADERS_RS=ON \
-DOPTION_BUILD_CLI=ON \
-DOPTION_BUILD_LOADERS_EXT=ON \
-DOPTION_BUILD_LOADERS_NODE=ON
```

Then compile:

```sh
make -j4
```

### 6. Verify the Build

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

### 7. Load a Rust Script

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

### 8. Call a Rust Function

Important: functions must be called using "call".

```sh
call add(1,2)
```
Output:
3

**Rust loader is correctly configured and working**