use metacall::{
    initialize, is_initialized,
    load::{self, Handle},
    metacall_handle_no_arg,
};

#[test]
fn metacall_handle() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    const SCRIPT1: &str = "function greet() { return 1 } \nmodule.exports = { greet }";
    const SCRIPT2: &str = "function greet() { return 2 } \nmodule.exports = { greet }";

    let mut handle1 = Handle::new();
    let mut handle2 = Handle::new();

    let result1 = load::from_memory(load::LoaderTag::NodeJS, SCRIPT1, Some(&mut handle1));
    let result2 = load::from_memory(load::LoaderTag::NodeJS, SCRIPT2, Some(&mut handle2));

    assert!(result1.is_ok());
    assert!(result2.is_ok());

    // Load first handle
    if result1.is_ok() {
        let out = metacall_handle_no_arg::<f64>(&mut handle1, "greet").unwrap();
        assert_eq!(out, 1.0, "Testing greet 1");
    }

    // Load second handle
    if result2.is_ok() {
        let out = metacall_handle_no_arg::<f64>(&mut handle2, "greet").unwrap();
        assert_eq!(out, 2.0, "Testing greet 2");
    }
}
