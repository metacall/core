use metacall::{loaders, switch, MetaCallLoaderError};
use std::env;

#[test]
fn invalid_loaders() {
    let _d = switch::initialize().unwrap();

    let scripts_dir = env::current_dir().unwrap().join("tests/scripts");
    let inavlid_file = scripts_dir.join("whatever.yeet");
    let valid_file = scripts_dir.join("script.js");

    if let Err(MetaCallLoaderError::FileNotFound(_)) =
        loaders::from_single_file("random", inavlid_file)
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FileNotFound` error variant!");
    }

    if let Err(MetaCallLoaderError::FromFileFailure) =
        loaders::from_single_file("random", valid_file)
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromFileFailure` error variant!");
    }

    if let Err(MetaCallLoaderError::FromMemoryFailure) =
        loaders::from_memory("random", "Invalid code!")
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromMemoryFailure` error variant!");
    }
}
