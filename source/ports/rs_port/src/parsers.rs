use crate::{bindings::metacall_value_id, prelude::Any};
use std::ffi::c_void;

pub fn metacall_to_any(ret: *mut c_void) -> Any {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => Any::Null,
        (_, 0) => Any::from_boolean(ret),
        (_, 1) => Any::from_char(ret),
        (_, 2) => Any::from_short(ret),
        (_, 3) => Any::from_int(ret),
        (_, 4) => Any::from_long(ret),
        (_, 5) => Any::from_float(ret),
        (_, 6) => Any::from_double(ret),
        (_, 7) => Any::from_string(ret),
        (_, 8) => Any::from_buffer(ret),
        (_, 9) => Any::from_array(ret),
        (_, 10) => Any::from_map(ret),
        (_, 11) => Any::from_pointer(ret),
        (_, 12) => Any::from_future(ret),
        (_, 13) => Any::from_function(ret),
        (_, 14) => Any::from_null(ret),
        (_, 15) => Any::from_class(ret),
        (_, 16) => Any::from_object(ret),
        (_, 17) => Any::from_exception(ret),
        (_, 18) => Any::from_throwable(ret),
        _ => Any::Null,
    }
}

pub fn any_to_metacall(args: impl IntoIterator<Item = Any>) -> Vec<*mut c_void> {
    args.into_iter()
        .map(|arg| match arg {
            Any::Array(x) => Any::to_array(x),
            Any::Boolean(x) => Any::to_boolean(x),
            Any::Buffer(x) => Any::to_buffer(x),
            Any::Char(x) => Any::to_char(x),
            Any::Class(x) => Any::to_class(x),
            Any::Double(x) => Any::to_double(x),
            Any::Exception(x) => Any::to_exception(x),
            Any::Float(x) => Any::to_float(x),
            Any::Function(x) => Any::to_function(x),
            Any::Future(x) => Any::to_future(x),
            Any::Int(x) => Any::to_int(x),
            Any::Long(x) => Any::to_long(x),
            Any::Map(x) => Any::to_map(x),
            Any::Null => Any::to_null(),
            Any::Object(x) => Any::to_object(x),
            Any::Pointer(x) => Any::to_pointer(x),
            Any::Short(x) => Any::to_short(x),
            Any::String(x) => Any::to_str(x),
            Any::Throwable(x) => Any::to_throwable(x),
        })
        .collect()
}
