<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/master/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%; margin: 0 auto;" width="80" height="80">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**[METACALL](https://github.com/metacall/core)** is a library that allows calling functions, methods or procedures between programming languages. With **[METACALL](https://github.com/metacall/core)** you can transparently execute code from / to any programming language, for example, call TypeScript code from Rust.

# Install

MetaCall is a C plugin based library. This crate wraps the C library into Rust, so in order to make it work, you should install MetaCall binaries first ([click here](https://github.com/metacall/install) for additional info about the install script):
``` sh
bash <(curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh)
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
use metacall::{hooks, loaders, structs::Any, metacall};

fn main() {
    // Metacall automatically shuts down when it goes out of scope
    let _ = hooks::initialize().unwrap();

    loaders::from_file("ts", "sum.ts").unwrap();

    let sum = metacall("sum", [Any::Double(1.0), Any::Double(2.0)]).unwrap();

    println!("sum: {:?}", sum);
}
```
