use metacall::{
    hooks,
    inline::{node, py, ts},
};

#[test]
fn test_inline() {
    let _d = hooks::initialize().unwrap();

    py! {
        print("hello world")
    }

    node! {
        console.log("hello world");
    }

    ts! {
        console.log("hello world");
    }
}
