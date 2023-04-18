use metacall::{
    hooks, loaders, MetacallClass, MetacallException, MetacallFunction, MetacallFuture,
    MetacallNull, MetacallObject, MetacallPointer, MetacallThrowable, MetacallValue,
};
use std::{collections::HashMap, env, fmt::Debug};

fn generate_test<T: MetacallValue + PartialEq + Debug + Clone>(
    name: impl ToString,
    expected: T,
) -> T {
    let test = if !(Box::new(expected.clone()) as Box<dyn MetacallValue>).is::<MetacallNull>() {
        ::metacall::metacall::<T>(name, [expected.clone()])
    } else {
        ::metacall::metacall_no_arg::<T>(name)
    };

    match test {
        Ok(v) => {
            if v != expected {
                invalid_return_value(expected.clone(), v);
            }
        }
        Err(err) => invalid_return_type(expected.clone(), err),
    };

    expected
}
fn generate_test_custom_validation<T: MetacallValue + Debug>(
    name: impl ToString,
    expected_type: impl ToString,
    expected_value: impl MetacallValue + Clone,
    validator: impl FnOnce(T),
) {
    let expected_value = Box::new(expected_value) as Box<dyn MetacallValue>;
    let test = if !expected_value.is::<MetacallNull>() {
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
    generate_test::<bool>("return_the_argument_py", false);
    generate_test::<bool>("return_the_argument_py", true);
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
    generate_test::<String>(
        "return_the_argument_py",
        generate_test::<String>("return_the_argument_py", String::from("hi there!")),
    );
}
fn test_buffer() {
    generate_test::<Vec<i8>>(
        "return_the_argument_py",
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
        "return_the_argument_py",
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
        "return_the_argument_py",
        "array",
        expected_array.clone(),
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
    let expected_value = String::from("hi there!");

    generate_test_custom_validation::<MetacallPointer>(
        "return_the_argument_py",
        "pointer",
        MetacallPointer::new(expected_value.clone()).unwrap(),
        |pointer| {
            let receieved_value = pointer.get_value::<String>().unwrap();

            if receieved_value != expected_value {
                invalid_return_value(expected_value, receieved_value)
            }
        },
    );
}
fn test_future() {
    fn validate(upper_result: Box<dyn MetacallValue>, upper_data: Box<dyn MetacallValue>) {
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
        |upper_future| {
            generate_test_custom_validation::<MetacallFuture>(
                "return_the_argument_py",
                "future",
                upper_future,
                move |future| {
                    fn resolve(result: Box<dyn MetacallValue>, data: Box<dyn MetacallValue>) {
                        validate(result, data);
                    }

                    future.then(resolve).data(String::from("data")).await_fut();
                },
            );
        },
    );
    generate_test_custom_validation::<MetacallFuture>(
        "test_future_reject",
        "future",
        MetacallNull(),
        |upper_future| {
            generate_test_custom_validation::<MetacallFuture>(
                "return_the_argument_py",
                "future",
                upper_future,
                move |future| {
                    fn reject(result: Box<dyn MetacallValue>, data: Box<dyn MetacallValue>) {
                        validate(result, data);
                    }

                    future.catch(reject).data(String::from("data")).await_fut();
                },
            );
        },
    );
}
fn test_function() {
    generate_test_custom_validation::<MetacallFunction>(
        "test_function",
        "function",
        MetacallNull(),
        |upper_function| {
            generate_test_custom_validation::<MetacallFunction>(
                "return_the_argument_py",
                "function",
                upper_function,
                move |function| {
                    let ret = function.call_no_arg::<String>().unwrap();
                    if ret.as_str() == "hi there!" {
                    } else {
                        invalid_return_value("hi there!", ret);
                    }
                },
            );
        },
    );
}
fn test_null() {
    metacall::metacall::<MetacallNull>("return_the_argument_py", [MetacallNull()]).unwrap();
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
        |upper_class| {
            generate_test_custom_validation::<MetacallClass>(
                "return_the_argument_py",
                "class",
                upper_class,
                class_test_inner,
            );
        },
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
        |upper_object| {
            generate_test_custom_validation::<MetacallObject>(
                "return_the_argument_py",
                "object",
                upper_object,
                object_test_inner,
            );
        },
    );
}
fn test_exception() {
    // generate_test_custom_validation::<MetacallException>(
    //     "test_exception",
    //     "exception",
    //     MetacallNull(),
    //     |upper_exception| {
    //         generate_test_custom_validation::<MetacallException>(
    //             "return_the_argument_js",
    //             "exception",
    //             upper_exception,
    //             move |exception| {
    //                 let exception_message = exception.get_message();
    //                 if exception_message.as_str() != "hi there!" {
    //                     invalid_return_value("hi there!", exception_message);
    //                 }
    //             },
    //         );
    //     },
    // );

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
    // generate_test_custom_validation::<MetacallThrowable>(
    //     "test_throwable",
    //     "throwable",
    //     MetacallNull(),
    //     |upper_throwable: MetacallThrowable| {
    //         generate_test_custom_validation::<MetacallThrowable>(
    //             "return_the_argument_py",
    //             "throwable",
    //             upper_throwable,
    //             |throwable| {
    //                 let exception_message = throwable
    //                     .get_value::<MetacallException>()
    //                     .unwrap()
    //                     .get_message();
    //                 if exception_message.as_str() != "hi there!" {
    //                     invalid_return_value("hi there!", exception_message);
    //                 }
    //             },
    //         );
    //     },
    // );

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
fn metacall() {
    let _d = hooks::initialize().unwrap();

    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let js_test_file = tests_dir.join("script.js");
    let c_test_file = tests_dir.join("script.c");
    let py_test_file = tests_dir.join("script.py");

    if let Ok(_) = loaders::from_single_file("py", py_test_file) {
        test_buffer();
        test_class();
        test_object();
        test_pointer();
    }
    if let Ok(_) = loaders::from_single_file("c", c_test_file) {
        test_char();
        test_double();
        test_float();
        test_int();
        test_long();
        test_short();
    }
    if let Ok(_) = loaders::from_single_file("node", js_test_file) {
        test_array();
        test_bool();
        test_exception();
        test_function();
        test_map();
        test_null();
        test_string();
        test_throwable();
        test_future();
    }
}
