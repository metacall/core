# Deno Rust plugin

This is a simple plugin to demonstrate calling Rust code from within Deno.

See [rust_deno/test.ts](../rust_deno/test.ts) for a usage example of this plugin from within a Deno script.

## Setup

```sh
cargo update
cargo build
```

Output library goes in `target/debug/librust_deno_plugin.so`
