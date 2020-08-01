# Deno from Rust

This is some simple Rust code that starts up Deno and calls run_command() to run a TypeScript file

## Setup

* Keep in mind that you need to build [rust_deno_plugin](../rust_deno_plugin) FIRST in order to run `test.ts`

```sh
cargo update
cargo build
cargo run
```

It uses a forked version of Deno with the following changes:

## `cli/Cargo.toml` - Added the following lines in order to compile Deno as a library:
```toml
[lib]
name = "deno"
path = "main.rs"
```

## `cli/main.rs` - Changed L585 from:
```rust
async fn run_command(flags: Flags, script: String) -> Result<(), ErrBox> {
```
to
```rust
pub async fn run_command(flags: Flags, script: String) -> Result<(), ErrBox> {
```
in order to create a callable function from the outside of the Deno project.
