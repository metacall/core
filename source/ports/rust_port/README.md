<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/master/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%; margin: 0 auto;" width="80" height="80">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL** is a library that allows calling functions, methods or procedures between programming languages. With **METACALL** you can transparently execute code from / to any programming language, for example, call Python code from NodeJS.

# Install

Install MetaCall binaries first ([click here](https://github.com/metacall/install) for additional info about the install script):
``` sh
bash <(curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh)
```

Then install MetaCall Rust package through Cargo:
``` sh
metacall cargo install metacall # Not implemented yet
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
fn main() {
	let _d = defer(|| metacall::destroy());

	match metacall::initialize() {
		Err(e) => { println!("{}", e); panic!(); },
		_ => { println!("MetaCall initialized") }
	}

	let scripts : Vec<String> = vec!["sum.ts".to_string()];

	match metacall::load_from_file("ts".to_string(), scripts) {
		Err(e) => { println!("{}", e); panic!(); },
		_ => ()
	}

	match metacall::metacall("sum".to_string(),
							vec![
								metacall::Any::Double(1.0),
								metacall::Any::Double(2.0)
								])
	{
		Err(e) => { println!("{}", e); panic!(); },
		Ok(ret) => { println!("{:?}", ret); }
  }
}
```

``` sh
metacall main.rs # Not implemented yet
```
