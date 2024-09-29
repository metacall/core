use crate::{
    bindings::metacall_value_id,
    types::{
        MetacallClass, MetacallException, MetacallFunction, MetacallFuture, MetacallNull,
        MetacallObject, MetacallPointer, MetacallThrowable, MetacallValue,
    },
};
use std::{collections::HashMap, ffi::c_void, fmt::Debug};

fn metacallobj_result_wrap<T: MetacallValue>(
    v: Result<T, Box<dyn MetacallValue>>,
) -> Box<dyn MetacallValue> {
    match v {
        Ok(obj) => Box::new(obj) as Box<dyn MetacallValue>,
        Err(original) => original,
    }
}

pub fn raw_to_metacallobj<T: MetacallValue>(ret: *mut c_void) -> Result<T, Box<dyn MetacallValue>> {
    let null = MetacallNull();

    if ret.is_null() {
        if <T>::get_metacall_id() != 14 {
            return Err(metacallobj_result_wrap(Ok(null)));
        } else {
            return Ok(<T>::from_metacall_raw(ret).unwrap());
        }
    }

    if unsafe { metacall_value_id(ret) } == T::get_metacall_id() {
        <T>::from_metacall_raw(ret)
    } else {
        Err(raw_to_metacallobj_untyped::<T>(ret))
    }
}
pub fn raw_to_metacallobj_leak<T: MetacallValue>(
    ret: *mut c_void,
) -> Result<T, Box<dyn MetacallValue>> {
    let null = MetacallNull();

    if ret.is_null() {
        if <T>::get_metacall_id() != 14 {
            return Err(metacallobj_result_wrap(Ok(null)));
        } else {
            return Ok(<T>::from_metacall_raw(ret).unwrap());
        }
    }

    if unsafe { metacall_value_id(ret) } == T::get_metacall_id() {
        <T>::from_metacall_raw_leak(ret)
    } else {
        Err(raw_to_metacallobj_untyped_leak::<T>(ret))
    }
}

pub fn raw_to_metacallobj_untyped<T: Debug + 'static>(ret: *mut c_void) -> Box<dyn MetacallValue> {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => metacallobj_result_wrap(MetacallNull::from_metacall_raw(ret)),
        (_, 0) => metacallobj_result_wrap(bool::from_metacall_raw(ret)),
        (_, 1) => metacallobj_result_wrap(char::from_metacall_raw(ret)),
        (_, 2) => metacallobj_result_wrap(i16::from_metacall_raw(ret)),
        (_, 3) => metacallobj_result_wrap(i32::from_metacall_raw(ret)),
        (_, 4) => metacallobj_result_wrap(i64::from_metacall_raw(ret)),
        (_, 5) => metacallobj_result_wrap(f32::from_metacall_raw(ret)),
        (_, 6) => metacallobj_result_wrap(f64::from_metacall_raw(ret)),
        (_, 7) => metacallobj_result_wrap(String::from_metacall_raw(ret)),
        (_, 8) => metacallobj_result_wrap(<Vec<i8>>::from_metacall_raw(ret)),
        (_, 9) => metacallobj_result_wrap(<Vec<MetacallNull>>::from_metacall_raw(ret)),
        (_, 10) => metacallobj_result_wrap(<HashMap<String, MetacallNull>>::from_metacall_raw(ret)),
        (_, 11) => metacallobj_result_wrap(<MetacallPointer>::from_metacall_raw(ret)),
        (_, 12) => {
            metacallobj_result_wrap::<MetacallFuture<T>>(MetacallFuture::from_metacall_raw(ret))
        }
        (_, 13) => metacallobj_result_wrap(MetacallFunction::from_metacall_raw(ret)),
        (_, 14) => metacallobj_result_wrap(MetacallNull::from_metacall_raw(ret)),
        (_, 15) => metacallobj_result_wrap(MetacallClass::from_metacall_raw(ret)),
        (_, 16) => metacallobj_result_wrap(MetacallObject::from_metacall_raw(ret)),
        (_, 17) => metacallobj_result_wrap(MetacallException::from_metacall_raw(ret)),
        (_, 18) => metacallobj_result_wrap(MetacallThrowable::from_metacall_raw(ret)),
        _ => metacallobj_result_wrap(MetacallNull::from_metacall_raw(ret)),
    }
}
pub fn raw_to_metacallobj_untyped_leak<T: Debug + 'static>(
    ret: *mut c_void,
) -> Box<dyn MetacallValue> {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => metacallobj_result_wrap(MetacallNull::from_metacall_raw_leak(ret)),
        (_, 0) => metacallobj_result_wrap(bool::from_metacall_raw_leak(ret)),
        (_, 1) => metacallobj_result_wrap(char::from_metacall_raw_leak(ret)),
        (_, 2) => metacallobj_result_wrap(i16::from_metacall_raw_leak(ret)),
        (_, 3) => metacallobj_result_wrap(i32::from_metacall_raw_leak(ret)),
        (_, 4) => metacallobj_result_wrap(i64::from_metacall_raw_leak(ret)),
        (_, 5) => metacallobj_result_wrap(f32::from_metacall_raw_leak(ret)),
        (_, 6) => metacallobj_result_wrap(f64::from_metacall_raw_leak(ret)),
        (_, 7) => metacallobj_result_wrap(String::from_metacall_raw_leak(ret)),
        (_, 8) => metacallobj_result_wrap(<Vec<i8>>::from_metacall_raw_leak(ret)),
        (_, 9) => metacallobj_result_wrap(<Vec<MetacallNull>>::from_metacall_raw_leak(ret)),
        (_, 10) => {
            metacallobj_result_wrap(<HashMap<String, MetacallNull>>::from_metacall_raw_leak(ret))
        }
        (_, 11) => metacallobj_result_wrap(<MetacallPointer>::from_metacall_raw_leak(ret)),
        (_, 12) => metacallobj_result_wrap::<MetacallFuture<T>>(
            MetacallFuture::from_metacall_raw_leak(ret),
        ),
        (_, 13) => metacallobj_result_wrap(MetacallFunction::from_metacall_raw_leak(ret)),
        (_, 14) => metacallobj_result_wrap(MetacallNull::from_metacall_raw_leak(ret)),
        (_, 15) => metacallobj_result_wrap(MetacallClass::from_metacall_raw_leak(ret)),
        (_, 16) => metacallobj_result_wrap(MetacallObject::from_metacall_raw_leak(ret)),
        (_, 17) => metacallobj_result_wrap(MetacallException::from_metacall_raw_leak(ret)),
        (_, 18) => metacallobj_result_wrap(MetacallThrowable::from_metacall_raw_leak(ret)),
        _ => metacallobj_result_wrap(MetacallNull::from_metacall_raw_leak(ret)),
    }
}

pub fn metacallobj_to_raw(arg: impl MetacallValue) -> *mut c_void {
    arg.into_metacall_raw()
}
pub fn metacallobj_to_raw_args(
    args: impl IntoIterator<Item = impl MetacallValue>,
) -> Vec<*mut c_void> {
    args.into_iter()
        .map(|arg| arg.into_metacall_raw())
        .collect::<Vec<*mut c_void>>()
}
