use metacall::{
    initialize,
    inline::{node, py, ts},
    is_initialized, load,
};

#[test]
fn inlines() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    if load::from_memory("py", "").is_ok() {
        py! {
            print("hello world")
        }
    }
    if load::from_memory("py", "").is_ok() {
        py! {print("hello world")}
    }

    if load::from_memory("node", "").is_ok() {
        node! {
            console.log("hello world");
        }
    }
    if load::from_memory("node", "").is_ok() {
        node! {console.log("hello world")}
    }

    if load::from_memory("ts", "").is_ok() {
        ts! {
            console.log("hello world");
        }
    }
    if load::from_memory("ts", "").is_ok() {
        ts! {console.log("hello world")}
    }
}
