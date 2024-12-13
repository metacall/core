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

# Example

`sum.ts`
``` javascript
export function sum(a: number, b: number): number {
	return a + b;
}
```

`main.rs`
``` rust
use metacall::{switch, metacall, loaders};

fn main() {
    // Initialize MetaCall at the top
    let _metacall = switch::initialize().unwrap();
     
    // Load the file
    load::from_single_file("ts", "sum.ts").unwrap();

    // Call the sum function
    let sum = metacall::<f64>("sum", [1.0, 2.0]).unwrap();

    assert_eq!(sum, 3.0);
}
```
