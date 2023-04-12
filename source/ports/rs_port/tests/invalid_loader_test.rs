use metacall::{
    hooks, loaders,
};
use std::{env};

#[test]
fn invalid_loader_test() {
    let _d = hooks::initialize().unwrap();

    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let test_file = tests_dir.join("whatever.yeet");

    loaders::from_file("random", [test_file]).unwrap();
}
