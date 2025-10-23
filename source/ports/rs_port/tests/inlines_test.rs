use metacall::{
    initialize,
    inline::{node, py, ts},
    is_initialized,
    load::{self, Tag},
};

#[test]
fn inlines() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    if load::from_memory(Tag::Python, "").is_ok() {
        py! {
            print("hello world")
        }
    }
    if load::from_memory(Tag::Python, "").is_ok() {
        py! {print("hello world")}
    }

    if load::from_memory(Tag::NodeJS, "").is_ok() {
        node! {
            console.log("hello world");
        }
    }
    if load::from_memory(Tag::NodeJS, "").is_ok() {
        node! {console.log("hello world")}
    }

    if load::from_memory(Tag::TypeScript, "").is_ok() {
        ts! {
            console.log("hello world");
        }
    }
    if load::from_memory(Tag::TypeScript, "").is_ok() {
        ts! {console.log("hello world")}
    }
}
