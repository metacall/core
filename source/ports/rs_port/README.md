<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/master/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%; margin: 0 auto;" width="80" height="80">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**[METACALL](https://github.com/metacall/core)** is a library that allows calling functions, methods or procedures between programming languages. With **[METACALL](https://github.com/metacall/core)** you can transparently execute code from / to any programming language, for example, call TypeScript code from Rust.

# Install

MetaCall is a C plugin based library. This crate wraps the C library into Rust, so in order to make it work, you should install MetaCall binaries first ([click here](https://github.com/metacall/install) for installing it on other platforms):
``` sh
curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | sh
```

# Linking

If your project uses MetaCall in a folder that is not in the system path, we encourage to use [`metacall-sys`](https://crates.io/crates/metacall-sys) crate as a [`build-dependecy`](https://doc.rust-lang.org/cargo/reference/specifying-dependencies.html#build-dependencies). By this way you will be able to locate and link MetaCall directly in your build system. For example:

`Cargo.toml`:
```toml
[build-dependencies]
metacall-sys = "0.1.2"
```

`build.rs`:
```rust
fn main() {
    // Find MetaCall library
    metacall_sys::build();
}
```

# Example

`sum.ts`
``` javascript
export function sum(a: number, b: number): number {
	return a + b;
}
```

`main.rs`
``` rust
use metacall::{initialize, metacall, load};

fn main() {
    // Initialize MetaCall at the top
    let _metacall = initialize().unwrap();
     
    // Load the file
    load::from_single_file("ts", "sum.ts").unwrap();

    // Call the sum function
    let sum = metacall::<f64>("sum", [1.0, 2.0]).unwrap();

    assert_eq!(sum, 3.0);
}
```
