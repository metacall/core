use metacall::{
    hooks, loaders,
    prelude::{
        MetacallClass, MetacallException, MetacallFuture, MetacallNull, MetacallObject,
        MetacallObjectProtocol, MetacallPointer, MetacallThrowable,
    },
};
use std::{collections::HashMap, env, fmt::Debug};

fn generate_test<T: MetacallObjectProtocol + PartialEq + Debug + Clone>(
    name: impl ToString,
    expected: T,
) {
    let test =
        if !(Box::new(expected.clone()) as Box<dyn MetacallObjectProtocol>).is::<MetacallNull>() {
            ::metacall::metacall::<T>(name, [expected.clone()])
        } else {
            ::metacall::metacall_no_arg::<T>(name)
        };

    match test {
        Ok(v) => {
            if v != expected {
                invalid_return_value(expected, v);
            }
        }
        Err(err) => invalid_return_type(expected, err),
    };
}
fn generate_test_custom_validation<T: MetacallObjectProtocol + Debug>(
    name: impl ToString,
    expected_type: impl ToString,
    expected_value: impl MetacallObjectProtocol + Clone,
    validator: impl Fn(T),
) {
    let test = if !(Box::new(expected_value.clone()) as Box<dyn MetacallObjectProtocol>)
        .is::<MetacallNull>()
    {
        ::metacall::metacall::<T>(name, [expected_value])
    } else {
        ::metacall::metacall_no_arg::<T>(name)
    };

    match test {
        Ok(v) => validator(v),
        Err(err) => invalid_return_type(expected_type.to_string(), err),
    };
}

fn invalid_return_type(expected: impl Debug, received: impl Debug) {
    panic!(
        "Invalid return type! Expected: `{:#?}` but received `{:#?}`.",
        expected, received
    );
}
fn invalid_return_value(expected: impl Debug, received: impl Debug) {
    panic!(
        "Invalid return value! Expected: `{:#?}` but received `{:#?}`.",
        expected, received
    );
}

fn test_bool() {
    generate_test::<bool>("test_bool", false);
    generate_test::<bool>("test_bool", true);
}
fn test_char() {
    generate_test::<char>("test_char", 'A');
    generate_test::<char>("test_char", 'Z');
}
fn test_short() {
    generate_test::<i16>("test_short", 12345 as i16);
}
fn test_int() {
    generate_test::<i32>("test_int", 12345 as i32);
}
fn test_long() {
    generate_test::<i64>("test_long", 12345 as i64);
}
fn test_float() {
    generate_test::<f32>("test_float", 1.2345 as f32);
}
fn test_double() {
    generate_test::<f64>("test_double", 1.2345 as f64);
}
fn test_string() {
    generate_test::<String>("test_string", String::from("hi there!"));
}
fn test_buffer() {
    generate_test::<Vec<i8>>(
        "test_buffer",
        String::from("hi there!")
            .as_bytes()
            .iter()
            .map(|&b| b as i8)
            .collect(),
    );
}
fn test_map() {
    let mut expected_hashmap = HashMap::new();
    expected_hashmap.insert(String::from("hi"), String::from("there!"));
    expected_hashmap.insert(String::from("hello"), String::from("world"));

    generate_test_custom_validation::<HashMap<String, String>>(
        "test_map",
        "map",
        expected_hashmap.clone(),
        move |hashmap| {
            for (expected_key, expected_value) in expected_hashmap.iter() {
                if let Some(received_value) = hashmap.get(expected_key) {
                    if received_value != expected_value {
                        invalid_return_value(expected_value, received_value);
                    }
                } else {
                    invalid_return_value(format!("'{}' for key", expected_key), "Null");
                }
            }
        },
    );
}
fn test_array() {
    let expected_array = vec![String::from("hi"), String::from("there!")];

    generate_test_custom_validation::<Vec<String>>(
        "test_array",
        "array",
        MetacallNull(),
        |array| {
            for (index, expected_value) in expected_array.iter().enumerate() {
                if &array[index] != expected_value {
                    invalid_return_value(
                        format!("'{}' as the {}th element", expected_value, index),
                        &array[index],
                    );
                }
            }
        },
    );
}
fn test_pointer() {
    metacall::metacall::<MetacallPointer>(
        "test_pointer",
        [MetacallPointer::new(Box::new(String::from("hi there!"))).unwrap()],
    )
    .unwrap();
}
fn test_future() {
    fn validate(
        upper_result: Box<dyn MetacallObjectProtocol>,
        upper_data: Box<dyn MetacallObjectProtocol>,
    ) {
        match upper_data.downcast::<String>() {
            Ok(ret) => {
                if ret.as_str() != "data" {
                    invalid_return_value("data", ret)
                }
            }
            Err(original) => {
                invalid_return_type("'string' for the data", original);
            }
        }

        match upper_result.downcast::<String>() {
            Ok(ret) => {
                if ret.as_str() != "hi there!" {
                    invalid_return_value("hi there!", ret)
                }
            }
            Err(original) => {
                invalid_return_type("'funtion' for the result", original);
            }
        }
    }

    generate_test_custom_validation::<MetacallFuture>(
        "test_future_resolve",
        "future",
        MetacallNull(),
        |future| {
            fn resolve(
                result: Box<dyn MetacallObjectProtocol>,
                data: Box<dyn MetacallObjectProtocol>,
            ) {
                validate(result, data);
            }

            future.await_fut(Some(resolve), None, Some(String::from("data")));
        },
    );
    generate_test_custom_validation::<MetacallFuture>(
        "test_future_reject",
        "future",
        MetacallNull(),
        |future| {
            fn reject(
                result: Box<dyn MetacallObjectProtocol>,
                data: Box<dyn MetacallObjectProtocol>,
            ) {
                validate(result, data);
            }

            future.await_fut(None, Some(reject), Some(String::from("data")));
        },
    );
}
// fn test_function() {
//     generate_test_custom_validation::<MetacallFunction>("test_function", "function", |function| {
//         let ret = function.call_no_arg::<String>().unwrap();
//         if ret.as_str() == "hi there!" {
//         } else {
//             invalid_return_value("hi there!", ret);
//         }
//     });
// }
fn test_null() {
    metacall::metacall_no_arg::<MetacallNull>("test_null").unwrap();
}
fn class_test_inner(class: MetacallClass) {
    let attr = class.get_attribute::<String>("hi").unwrap();
    if attr.as_str() != "there!" {
        invalid_return_value("there!", attr);
    }

    class
        .set_attribute("hi2", String::from("there!2!"))
        .unwrap();
    let attr = class.get_attribute::<String>("hi2").unwrap();
    if attr.as_str() != "there!2!" {
        invalid_return_value("there!2!", attr);
    }

    let ret = class.call_method_no_arg::<String>("hi_function").unwrap();
    if ret != "there!" {
        invalid_return_value("there!", ret)
    }

    let new_obj = class.create_object_no_arg("test_class_obj").unwrap();
    object_test_inner(new_obj);
}
fn test_class() {
    generate_test_custom_validation::<MetacallClass>(
        "test_class",
        "class",
        MetacallNull(),
        class_test_inner,
    );

    class_test_inner(MetacallClass::from_name("TestClass").unwrap());
}
fn object_test_inner(object: MetacallObject) {
    object
        .set_attribute("hi2", String::from("there!2!"))
        .unwrap();
    let attr = object.get_attribute::<String>("hi2").unwrap();
    if attr.as_str() != "there!2!" {
        invalid_return_value("there!", attr);
    }

    let ret = object.call_method_no_arg::<String>("get_hi").unwrap();
    if ret.as_str() == "there!" {
    } else {
        invalid_return_value("there!", ret);
    }
}
fn test_object() {
    generate_test_custom_validation::<MetacallObject>(
        "test_object",
        "object",
        MetacallNull(),
        object_test_inner,
    );
}
fn test_exception() {
    generate_test_custom_validation::<MetacallException>(
        "test_exception",
        "exception",
        MetacallNull(),
        move |exception| {
            let exception_message = exception.get_message();
            if exception_message.as_str() != "hi there!" {
                invalid_return_value("hi there!", exception_message);
            }
        },
    );
}
fn test_throwable() {
    generate_test_custom_validation::<MetacallThrowable>(
        "test_throwable",
        "throwable",
        MetacallNull(),
        |throwable| {
            let exception_message = throwable
                .get_value::<MetacallException>()
                .unwrap()
                .get_message();
            if exception_message.as_str() != "hi there!" {
                invalid_return_value("hi there!", exception_message);
            }
        },
    );
}

#[test]
fn return_type_test() {
    let _d = hooks::initialize().unwrap();

    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let js_test_file = tests_dir.join("return_type_test.js");
    let c_test_file = tests_dir.join("return_type_test.c");
    let py_test_file = tests_dir.join("return_type_test.py");

    loaders::from_file("node", [js_test_file]).unwrap();
    loaders::from_file("c", [c_test_file]).unwrap();
    loaders::from_file("py", [py_test_file]).unwrap();

    test_bool();
    test_char();
    test_short();
    test_int();
    test_long();
    test_float();
    test_double();
    test_string();
    test_buffer();
    test_array();
    test_map();
    test_pointer();
    // test_function();
    test_null();
    test_class();
    test_object();
    test_exception();
    test_throwable();
    test_future();
}
