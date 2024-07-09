use metacall::{
    inline::{node, py, ts},
    loaders, switch,
};

#[test]
fn inlines() {
    let _d = switch::initialize().unwrap();

    if loaders::from_memory("py", "").is_ok() {
        py! {
            print("hello world")
        }
    }
    if loaders::from_memory("py", "").is_ok() {
        py! {print("hello world")}
    }

    if loaders::from_memory("node", "").is_ok() {
        node! {
            console.log("hello world");
        }
    }
    if loaders::from_memory("node", "").is_ok() {
        node! {console.log("hello world")}
    }

    if loaders::from_memory("ts", "").is_ok() {
        ts! {
            console.log("hello world");
        }
    }
    if loaders::from_memory("ts", "").is_ok() {
        ts! {console.log("hello world")}
    }
}
