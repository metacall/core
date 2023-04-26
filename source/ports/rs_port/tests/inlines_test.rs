use metacall::{
    hooks,
    loaders,
    inline::{node, py, ts},
};

#[test]
fn inlines() {
    let _d = hooks::initialize().unwrap();

    if loaders::from_memory("py", "").is_ok() {
        py! {
            print("hello world")
        }
    }

    if loaders::from_memory("node", "").is_ok() {
        node! {
            console.log("hello world");
        }
    }

    if loaders::from_memory("ts", "").is_ok() {
        ts! {
            console.log("hello world");
        }
    }
}
