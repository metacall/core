use metacall::{
    initialize, is_initialized,
    load::{self, Tag},
    MetaCallClass, MetaCallException, MetaCallFunction, MetaCallFuture, MetaCallNull,
    MetaCallObject, MetaCallPointer, MetaCallThrowable, MetaCallValue,
};
use std::{any::Any, collections::HashMap, env, fmt::Debug};

fn generate_test<T: MetaCallValue + PartialEq + Debug + Clone>(
    name: impl ToString,
    expected: T,
) -> T {
    let test = if !(Box::new(expected.clone()) as Box<dyn MetaCallValue>).is::<MetaCallNull>() {
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
fn generate_test_custom_validation<T: MetaCallValue + Debug>(
    name: impl ToString,
    expected_type: impl ToString,
    expected_value: impl MetaCallValue + Clone,
    validator: impl FnOnce(T),
) {
    let expected_value_boxed = Box::new(expected_value.clone()) as Box<dyn MetaCallValue>;
    let test = if !expected_value_boxed.is::<MetaCallNull>() {
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
    generate_test::<i16>("test_short", 12345_i16);
}
fn test_int() {
    generate_test::<i32>("test_int", 12345_i32);
}
fn test_long() {
    generate_test::<i64>("test_long", 12345_i64);
}
fn test_float() {
    generate_test::<f32>("test_float", 1.2345_f32);
}
fn test_double() {
    generate_test::<f64>("test_double", 1.2345_f64);
}
fn test_mixed_numbers() {
    let result = ::metacall::metacall::<i64>(
        "test_mixed_numbers",
        [
            ::metacall::metacall_box(1 as i16),
            ::metacall::metacall_box(2 as i32),
            ::metacall::metacall_box(3 as i64),
        ],
    );

    assert!(result.is_ok());

    if let Ok(ret) = result {
        assert_eq!(ret, 6_i64)
    }
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

    generate_test_custom_validation::<MetaCallPointer>(
        "return_the_argument_py",
        "pointer",
        MetaCallPointer::new(expected_value.clone()),
        |pointer| {
            let receieved_value = pointer.get_value::<String>().unwrap();

            if receieved_value != expected_value {
                invalid_return_value(expected_value, receieved_value)
            }
        },
    );
}
fn test_future() {
    fn validate(upper_result: Box<dyn MetaCallValue>, upper_data: Option<Box<dyn Any>>) {
        match upper_data {
            Some(data) => match data.downcast::<String>() {
                Ok(ret) => {
                    if ret.as_str() != "data" {
                        invalid_return_value("data", ret)
                    }
                }
                Err(original) => {
                    invalid_return_type("'string' for the data", original);
                }
            },
            None => println!("user_data is None."),
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

    generate_test_custom_validation::<MetaCallFuture>(
        "test_future_resolve",
        "future",
        MetaCallNull(),
        move |future| {
            fn resolve(
                result: Box<dyn MetaCallValue>,
                data: Option<Box<dyn Any>>,
            ) -> Box<dyn MetaCallValue> {
                validate(result.clone(), data);
                result.clone()
            }

            future.then(resolve).data(String::from("data")).await_fut();
        },
    );
    generate_test_custom_validation::<MetaCallFuture>(
        "test_future_reject",
        "future",
        MetaCallNull(),
        move |future| {
            fn reject(
                result: Box<dyn MetaCallValue>,
                data: Option<Box<dyn Any>>,
            ) -> Box<dyn MetaCallValue> {
                validate(result.clone(), data);
                result.clone()
            }

            future.catch(reject).data(String::from("data")).await_fut();
        },
    );
}
fn test_function() {
    generate_test_custom_validation::<MetaCallFunction>(
        "test_function",
        "function",
        MetaCallNull(),
        |upper_function| {
            generate_test_custom_validation::<MetaCallFunction>(
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
    metacall::metacall::<MetaCallNull>("return_the_argument_py", [MetaCallNull()]).unwrap();
}
fn class_test_inner(class: MetaCallClass) {
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
    generate_test_custom_validation::<MetaCallClass>(
        "test_class",
        "class",
        MetaCallNull(),
        |upper_class| {
            generate_test_custom_validation::<MetaCallClass>(
                "return_the_argument_py",
                "class",
                upper_class,
                class_test_inner,
            );
        },
    );

    class_test_inner(MetaCallClass::from_name("TestClass").unwrap());
}
fn object_test_inner(object: MetaCallObject) {
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
    generate_test_custom_validation::<MetaCallObject>(
        "test_object",
        "object",
        MetaCallNull(),
        |upper_object| {
            generate_test_custom_validation::<MetaCallObject>(
                "return_the_argument_py",
                "object",
                upper_object,
                object_test_inner,
            );
        },
    );
}
fn test_exception() {
    generate_test_custom_validation::<MetaCallException>(
        "test_exception",
        "exception",
        MetaCallNull(),
        |upper_exception| {
            generate_test_custom_validation::<MetaCallException>(
                "return_the_argument_js",
                "exception",
                upper_exception,
                move |exception| {
                    let exception_message = exception.get_message();
                    if exception_message.as_str() != "hi there!" {
                        invalid_return_value("hi there!", exception_message);
                    }
                },
            );
        },
    );
}
fn test_throwable() {
    generate_test_custom_validation::<MetaCallThrowable>(
        "test_throwable",
        "throwable",
        MetaCallNull(),
        |throwable| {
            let exception_message = throwable
                .get_value::<MetaCallException>()
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
    let _d = initialize().unwrap();

    assert!(is_initialized());

    let tests_dir = env::current_dir().unwrap().join("tests/scripts");
    let js_test_file = tests_dir.join("script.js");
    let c_test_file = tests_dir.join("script.c");
    let py_test_file = tests_dir.join("script.py");
    let py_loaded = load::from_single_file(Tag::Python, py_test_file, None).is_ok();

    if py_loaded {
        test_buffer();
        test_class();
        test_object();
        test_pointer();
        test_array();
        test_bool();
        test_map();
        test_string();
        test_null();
    }
    if load::from_single_file(load::Tag::C, c_test_file, None).is_ok() {
        test_char();
        test_double();
        test_float();
        test_int();
        test_long();
        test_short();
        test_mixed_numbers();
    }
    if load::from_single_file(load::Tag::NodeJS, js_test_file, None).is_ok() {
        test_exception();
        test_throwable();
        test_future();
        if py_loaded {
            test_function();
        }
    }
}
