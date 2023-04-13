use metacall::{hooks, loaders, prelude::MetacallLoaderError};
use std::env;

#[test]
fn invalid_loader_test() {
    let _d = hooks::initialize().unwrap();

    let scripts_dir = env::current_dir().unwrap().join("tests/scripts");
    let inavlid_file = scripts_dir.join("whatever.yeet");
    let valid_file = scripts_dir.join("return_type_test.js");

    if let Err(MetacallLoaderError::FileNotFound(_)) = loaders::from_file("random", inavlid_file) {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FileNotFound` error variant!");
    }

    if let Err(MetacallLoaderError::FromFileFailure) = loaders::from_file("random", valid_file) {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromFileFailure` error variant!");
    }

    if let Err(MetacallLoaderError::FromMemoryFailure) =
        loaders::from_memory("random", "Invalid code!")
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromMemoryFailure` error variant!");
    }
}
