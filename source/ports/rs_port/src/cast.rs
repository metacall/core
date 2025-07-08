use crate::{
    bindings::metacall_value_id,
    types::{
        MetaCallClass, MetaCallException, MetaCallFunction, MetaCallFuture, MetaCallNull,
        MetaCallObject, MetaCallPointer, MetaCallThrowable, MetaCallValue,
    },
};
use std::{collections::HashMap, ffi::c_void};

fn metacallobj_result_wrap<T: MetaCallValue>(
    v: Result<T, Box<dyn MetaCallValue>>,
) -> Box<dyn MetaCallValue> {
    match v {
        Ok(obj) => Box::new(obj) as Box<dyn MetaCallValue>,
        Err(original) => original,
    }
}

pub fn raw_to_metacallobj<T: MetaCallValue>(ret: *mut c_void) -> Result<T, Box<dyn MetaCallValue>> {
    let null = MetaCallNull();

    if ret.is_null() {
        if <T>::get_metacall_id() != metacall_value_id::METACALL_NULL {
            return Err(metacallobj_result_wrap(Ok(null)));
        } else {
            return Ok(<T>::from_metacall_raw(ret).unwrap());
        }
    }

    if unsafe { metacall_value_id(ret) } == T::get_metacall_id() {
        <T>::from_metacall_raw(ret)
    } else {
        Err(raw_to_metacallobj_untyped(ret))
    }
}
pub fn raw_to_metacallobj_leak<T: MetaCallValue>(
    ret: *mut c_void,
) -> Result<T, Box<dyn MetaCallValue>> {
    let null = MetaCallNull();

    if ret.is_null() {
        if <T>::get_metacall_id() != metacall_value_id::METACALL_NULL {
            return Err(metacallobj_result_wrap(Ok(null)));
        } else {
            return Ok(<T>::from_metacall_raw(ret).unwrap());
        }
    }

    if unsafe { metacall_value_id(ret) } == T::get_metacall_id() {
        <T>::from_metacall_raw_leak(ret)
    } else {
        Err(raw_to_metacallobj_untyped_leak(ret))
    }
}

pub fn raw_to_metacallobj_untyped(ret: *mut c_void) -> Box<dyn MetaCallValue> {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => metacallobj_result_wrap(MetaCallNull::from_metacall_raw(ret)),
        (_, metacall_value_id::METACALL_BOOL) => {
            metacallobj_result_wrap(bool::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_CHAR) => {
            metacallobj_result_wrap(char::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_SHORT) => {
            metacallobj_result_wrap(i16::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_INT) => {
            metacallobj_result_wrap(i32::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_LONG) => {
            metacallobj_result_wrap(i64::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_FLOAT) => {
            metacallobj_result_wrap(f32::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_DOUBLE) => {
            metacallobj_result_wrap(f64::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_STRING) => {
            metacallobj_result_wrap(String::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_BUFFER) => {
            metacallobj_result_wrap(<Vec<i8>>::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_ARRAY) => {
            metacallobj_result_wrap(<Vec<MetaCallNull>>::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_MAP) => {
            metacallobj_result_wrap(<HashMap<String, MetaCallNull>>::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_PTR) => {
            metacallobj_result_wrap(<MetaCallPointer>::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_FUTURE) => {
            metacallobj_result_wrap(MetaCallFuture::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_FUNCTION) => {
            metacallobj_result_wrap(MetaCallFunction::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_NULL) => {
            metacallobj_result_wrap(MetaCallNull::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_CLASS) => {
            metacallobj_result_wrap(MetaCallClass::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_OBJECT) => {
            metacallobj_result_wrap(MetaCallObject::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_EXCEPTION) => {
            metacallobj_result_wrap(MetaCallException::from_metacall_raw(ret))
        }
        (_, metacall_value_id::METACALL_THROWABLE) => {
            metacallobj_result_wrap(MetaCallThrowable::from_metacall_raw(ret))
        }
        _ => metacallobj_result_wrap(MetaCallNull::from_metacall_raw(ret)),
    }
}
pub fn raw_to_metacallobj_untyped_leak(ret: *mut c_void) -> Box<dyn MetaCallValue> {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => metacallobj_result_wrap(MetaCallNull::from_metacall_raw_leak(ret)),
        (_, metacall_value_id::METACALL_BOOL) => {
            metacallobj_result_wrap(bool::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_CHAR) => {
            metacallobj_result_wrap(char::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_SHORT) => {
            metacallobj_result_wrap(i16::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_INT) => {
            metacallobj_result_wrap(i32::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_LONG) => {
            metacallobj_result_wrap(i64::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_FLOAT) => {
            metacallobj_result_wrap(f32::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_DOUBLE) => {
            metacallobj_result_wrap(f64::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_STRING) => {
            metacallobj_result_wrap(String::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_BUFFER) => {
            metacallobj_result_wrap(<Vec<i8>>::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_ARRAY) => {
            metacallobj_result_wrap(<Vec<MetaCallNull>>::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_MAP) => {
            metacallobj_result_wrap(<HashMap<String, MetaCallNull>>::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_PTR) => {
            metacallobj_result_wrap(<MetaCallPointer>::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_FUTURE) => {
            metacallobj_result_wrap(MetaCallFuture::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_FUNCTION) => {
            metacallobj_result_wrap(MetaCallFunction::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_NULL) => {
            metacallobj_result_wrap(MetaCallNull::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_CLASS) => {
            metacallobj_result_wrap(MetaCallClass::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_OBJECT) => {
            metacallobj_result_wrap(MetaCallObject::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_EXCEPTION) => {
            metacallobj_result_wrap(MetaCallException::from_metacall_raw_leak(ret))
        }
        (_, metacall_value_id::METACALL_THROWABLE) => {
            metacallobj_result_wrap(MetaCallThrowable::from_metacall_raw_leak(ret))
        }
        _ => metacallobj_result_wrap(MetaCallNull::from_metacall_raw_leak(ret)),
    }
}

pub fn metacallobj_to_raw(arg: impl MetaCallValue) -> *mut c_void {
    arg.into_metacall_raw()
}
pub fn metacallobj_to_raw_args(
    args: impl IntoIterator<Item = impl MetaCallValue>,
) -> Vec<*mut c_void> {
    args.into_iter()
        .map(|arg| arg.into_metacall_raw())
        .collect::<Vec<*mut c_void>>()
}

pub fn metacallobj_untyped_to_raw(ret: Box<dyn MetaCallValue>) -> Option<*mut c_void> {
    if let Some(v) = ret.downcast_ref::<bool>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<char>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<i16>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<i32>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<i64>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<f32>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<f64>() {
        return Some(v.into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<String>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<Vec<i8>>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<Vec<MetaCallNull>>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<HashMap<String, MetaCallNull>>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallPointer>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallFuture>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallFunction>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallNull>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallClass>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallObject>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallException>() {
        return Some(v.clone().into_metacall_raw());
    } else if let Some(v) = ret.downcast_ref::<MetaCallThrowable>() {
        return Some(v.clone().into_metacall_raw());
    }

    None
}

pub fn metacall_box(v: impl MetaCallValue) -> Box<dyn MetaCallValue> {
    Box::new(v) as Box<dyn MetaCallValue>
}
