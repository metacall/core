use metacall::{
    initialize,
    inline::{node, py, ts},
    is_initialized, load,
};

#[test]
fn inlines() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    if load::from_memory(load::LoaderTag::Python, "", None).is_ok() {
        py! {
            print("hello world")
        }
    }
    if load::from_memory(load::LoaderTag::Python, "", None).is_ok() {
        py! {print("hello world")}
    }

    if load::from_memory(load::LoaderTag::NodeJS, "", None).is_ok() {
        node! {
            console.log("hello world");
        }
    }
    if load::from_memory(load::LoaderTag::NodeJS, "", None).is_ok() {
        node! {console.log("hello world")}
    }

    if load::from_memory(load::LoaderTag::TypeScript, "", None).is_ok() {
        ts! {
            console.log("hello world");
        }
    }
    if load::from_memory(load::LoaderTag::TypeScript, "", None).is_ok() {
        ts! {console.log("hello world")}
    }
}
