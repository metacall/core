use metacall::{hooks, loaders, prelude::Any};
use std::{
    collections::HashMap,
    env,
    fmt::{Debug, Display},
};

macro_rules! generate_test {
    ($name:ident, $enum_variant:ident, $expected:expr) => {
        fn $name() {
            let test = ::metacall::metacall(stringify!($name), []).unwrap();
            match test {
                ::metacall::prelude::Any::$enum_variant(v) => {
                    let expected = $expected;

                    if v != expected {
                        invalid_return_value(expected, v);
                    }
                }
                _ => invalid_return_type(stringify!($name).replace("test_", ""), test),
            }
        }
    };
}
macro_rules! generate_test_custom_validation {
    ($name:ident, $enum_variant:ident, $expected:expr) => {
        fn $name() {
            let test = ::metacall::metacall(stringify!($name), []).unwrap();
            match test {
                ::metacall::prelude::Any::$enum_variant(v) => $expected(v),
                _ => invalid_return_type(stringify!($name).replace("test_", ""), test),
            }
        }
    };
}

fn invalid_return_type(expected: impl Display, received: impl Debug) {
    panic!(
        "Invalid return type! Expected: `{}` but received `{:#?}`.",
        expected, received
    );
}
fn invalid_return_value(expected: impl Debug, received: impl Debug) {
    panic!(
        "Invalid return value! Expected: `{:#?}` but received `{:#?}`.",
        expected, received
    );
}

generate_test!(test_bool_false, Boolean, false);
generate_test!(test_bool_true, Boolean, true);
generate_test!(test_char, Char, 'A');
generate_test!(test_short, Short, 12345 as i16);
generate_test!(test_int, Int, 12345 as i32);
generate_test!(test_long, Long, 12345 as i64);
generate_test!(test_float, Float, 1.2345 as f32);
generate_test!(test_double, Double, 1.2345 as f64);
generate_test!(test_string, String, "Hi there!");
// generate_test!(test_buffer, Buffer, "Hi there!");
generate_test_custom_validation!(test_map, Map, |hashmap: HashMap<Box<Any>, Box<Any>>| {
    if let Some(hi) = hashmap.get(&Box::new(Any::String(String::from("hi")))) {
        if let Any::String(v) = &**hi {
            if v.as_str() == "There" {
                if let Some(hello) = hashmap.get(&Box::new(Any::String(String::from("hello")))) {
                    if let Any::String(v) = &**hello {
                        if v.as_str() == "World" {
                        } else {
                            invalid_return_value("World", v);
                        }
                    } else {
                        invalid_return_type(
                            "'hello' key as `String`",
                            format!("'hello' key as `{:#?}`", hi),
                        );
                    }
                } else {
                    invalid_return_value("'hello' key", "Null");
                }
            } else {
                invalid_return_value("There", v);
            }
        } else {
            invalid_return_type("'hi' key as `String`", format!("'hi' key as `{:#?}`", hi));
        }
    } else {
        invalid_return_value("'hi' key", "Null");
    }
});

#[test]
fn return_type_test() {
    let _d = hooks::initialize().unwrap();

    let tests_dir = env::current_dir().unwrap().join("tests");
    let js_test_file = tests_dir.join("return_type_test.js");
    let c_test_file = tests_dir.join("return_type_test.c");

    loaders::from_file("node", [js_test_file]).unwrap();
    loaders::from_file("c", [c_test_file]).unwrap();

    test_bool_false();
    test_bool_true();
    test_char();
    test_short();
    test_int();
    test_long();
    test_float();
    test_double();
    test_string();
    // test_buffer();
    test_map();
}
