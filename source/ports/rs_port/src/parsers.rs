use crate::{
    bindings::metacall_value_id,
    match_object_protocol,
    prelude::{
        MetacallClass, MetacallException, MetacallFunction, MetacallFuture, MetacallNull,
        MetacallObject, MetacallObjectProtocol, MetacallPointer, MetacallThrowable,
    },
};
use std::{collections::HashMap, ffi::c_void};

pub fn metacallobj_wrap<T: MetacallObjectProtocol>(
    v: Result<T, Box<dyn MetacallObjectProtocol>>,
) -> Box<dyn MetacallObjectProtocol> {
    match v {
        Ok(obj) => Box::new(obj) as Box<dyn MetacallObjectProtocol>,
        Err(original) => original,
    }
}
pub fn implementer_to_traitobj(v: impl MetacallObjectProtocol) -> Box<dyn MetacallObjectProtocol> {
    Box::new(v) as Box<dyn MetacallObjectProtocol>
}

pub fn raw_to_metacallobj<T: MetacallObjectProtocol>(
    ret: *mut c_void,
) -> Result<T, Box<dyn MetacallObjectProtocol>> {
    let null = MetacallNull();

    if ret.is_null() {
        if <T>::get_metacall_id() != 14 {
            return Err(metacallobj_wrap(Ok(null)));
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
pub fn raw_to_metacallobj_untyped(ret: *mut c_void) -> Box<dyn MetacallObjectProtocol> {
    match (ret.is_null(), unsafe { metacall_value_id(ret) }) {
        (true, _) => metacallobj_wrap(MetacallNull::from_metacall_raw(ret)),
        (_, 0) => metacallobj_wrap(bool::from_metacall_raw(ret)),
        (_, 1) => metacallobj_wrap(char::from_metacall_raw(ret)),
        (_, 2) => metacallobj_wrap(i16::from_metacall_raw(ret)),
        (_, 3) => metacallobj_wrap(i32::from_metacall_raw(ret)),
        (_, 4) => metacallobj_wrap(i64::from_metacall_raw(ret)),
        (_, 5) => metacallobj_wrap(f32::from_metacall_raw(ret)),
        (_, 6) => metacallobj_wrap(f64::from_metacall_raw(ret)),
        (_, 7) => metacallobj_wrap(String::from_metacall_raw(ret)),
        (_, 8) => metacallobj_wrap(<Vec<i8>>::from_metacall_raw(ret)),
        (_, 9) => metacallobj_wrap(<Vec<Box<dyn MetacallObjectProtocol>>>::from_metacall_raw(
            ret,
        )),
        (_, 10) => metacallobj_wrap(
            <HashMap<String, Box<dyn MetacallObjectProtocol>>>::from_metacall_raw(ret),
        ),
        (_, 11) => metacallobj_wrap(<MetacallPointer>::from_metacall_raw(ret)),
        (_, 12) => metacallobj_wrap(MetacallFuture::from_metacall_raw(ret)),
        (_, 13) => metacallobj_wrap(MetacallFunction::from_metacall_raw(ret)),
        (_, 14) => metacallobj_wrap(MetacallNull::from_metacall_raw(ret)),
        (_, 15) => metacallobj_wrap(MetacallClass::from_metacall_raw(ret)),
        (_, 16) => metacallobj_wrap(MetacallObject::from_metacall_raw(ret)),
        (_, 17) => metacallobj_wrap(MetacallException::from_metacall_raw(ret)),
        (_, 18) => metacallobj_wrap(MetacallThrowable::from_metacall_raw(ret)),
        _ => metacallobj_wrap(MetacallNull::from_metacall_raw(ret)),
    }
}
pub fn metacallobj_to_raw(arg: impl MetacallObjectProtocol) -> (*mut c_void, impl FnOnce()) {
    let (ptr, value_drop) = arg.into_metacall_raw();
    (ptr, move || {
        if let Some(value_drop) = value_drop {
            value_drop();
        }
    })
}
pub fn metacallobj_to_raw_args(
    args: impl IntoIterator<Item = impl MetacallObjectProtocol>,
) -> (Vec<*mut c_void>, impl FnOnce()) {
    let mut value_drops = Vec::new();
    let ptr = args
        .into_iter()
        .map(|arg| {
            let (ptr, value_drop) = arg.into_metacall_raw();

            if let Some(value_drop) = value_drop {
                value_drops.push(value_drop);
            }

            ptr
        })
        .collect::<Vec<*mut c_void>>();

    (ptr.clone(), move || {
        for value_drop in value_drops.into_iter() {
            value_drop();
        }
    })
}

pub fn hashmap_key_with_downcast(key: Box<dyn MetacallObjectProtocol>) -> String {
    match_object_protocol!(key, {
        str: String => str,
        num: i16 => num.to_string(),
        num: i32 => num.to_string(),
        num: i64 => num.to_string(),
        num: f32 => num.to_string(),
        num: f64 => num.to_string(),
        _ =>  String::from("Invalid key!")
    })
}

pub fn new_raw_pointer<T>(pointer: T) -> *mut T {
    Box::into_raw(Box::new(pointer))
}
pub fn new_void_pointer<T>(pointer: T) -> *mut c_void {
    new_raw_pointer(pointer) as *mut c_void
}
pub fn pointer_to_box<T>(pointer: *mut c_void) -> Box<T> {
    unsafe { Box::from_raw(pointer as *mut T) }
}
