use metacall::{initialize, is_initialized, load, MetaCallLoaderError};
use std::env;

#[test]
fn invalid_loaders() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    let scripts_dir = env::current_dir().unwrap().join("tests/scripts");
    let inavlid_file = scripts_dir.join("whatever.yeet");
    let valid_file = scripts_dir.join("script.js");

    if let Err(MetaCallLoaderError::FileNotFound(_)) =
        load::from_single_file(load::LoaderTag::NodeJS, inavlid_file, None)
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FileNotFound` error variant!");
    }

    // We use JSM here because it is not implemented, it should fail
    if let Err(MetaCallLoaderError::FromFileFailure) =
        load::from_single_file(load::LoaderTag::JSM, valid_file, None)
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromFileFailure` error variant!");
    }

    if let Err(MetaCallLoaderError::FromMemoryFailure) =
        load::from_memory(load::LoaderTag::NodeJS, "Invalid code!", None)
    {
        // Everything Ok
    } else {
        panic!("Expected the loader fail with `FromMemoryFailure` error variant!");
    }
}
