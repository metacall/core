use super::{
    MetacallClass, MetacallException, MetacallFunction, MetacallFuture, MetacallNull,
    MetacallObject, MetacallPointer, MetacallThrowable,
};
use crate::{
    bindings::*,
    cstring,
    helpers::{MetacallClone, MetacallDowncast},
    match_metacall_value, parsers,
};
use std::{
    collections::HashMap,
    ffi::{c_char, c_int, c_void, CStr},
    fmt::Debug,
    slice,
};

/// Trait of any possible object in Metacall.
/// Checkout [match_metacall_value](match_metacall_value) macro for
/// matching trait objects of this trait. Let' see what types we can use with an example: ...
/// ```
/// // bool
/// metacall::metacall_untyped("x", [true, false]);
/// // char
/// metacall::metacall_untyped("x", ['A', 'Z']);
/// // short
/// metacall::metacall_untyped("x", [5 as i16]);
/// // int
/// metacall::metacall_untyped("x", [5 as i32]);
/// // long
/// metacall::metacall_untyped("x", [5 as i64]);
/// // float
/// metacall::metacall_untyped("x", [5.0 as f32]);
/// // double
/// metacall::metacall_untyped("x", [5.0 as f64]);
/// // string
/// metacall::metacall_untyped("x", [String::from("hi")]);
/// // buffer
/// metacall::metacall_untyped("x", [
///     String::from("hi")
///     .as_bytes()
///     .iter()
///     .map(|&b| b as i8)
///     .collect::<Vec<i8>>()
/// ]);
/// // array
/// metacall::metacall_untyped("x", [vec![5, 6]]);
/// // map
/// let mut hashmap = std::collections::HashMap::new();
/// hashmap.insert(String::from("hi"), String::from("there!"));
/// metacall::metacall_untyped("x", [hashmap]);
/// // pointer
/// metacall::metacall_untyped("x", [metacall::MetacallPointer::new(String::from("hi"))]);
/// // future?
/// // nope! you can't pass a future!
/// // function
/// metacall::metacall_untyped("x", [
///     metacall::metacall_no_arg::<metacall::MetacallFunction>("my_async_function").unwrap()
/// ]);
/// // null
/// metacall::metacall_untyped("x", [metacall::MetacallNull()]);
/// // class
/// metacall::metacall_untyped("x", [metacall::MetacallClass::from_name("MyClass").unwrap()]);
/// // object
/// let class = metacall::MetacallClass::from_name("MyClass").unwrap();
/// metacall::metacall_untyped("x", [class.create_object_no_arg("myObject").unwrap()]);
/// // exception
/// metacall::metacall_untyped("x", [
///     metacall::metacall_no_arg::<metacall::MetacallException>("my_function").unwrap()
/// ]);
/// // throwable?
/// // nope! you can't pass a throwable!
/// ```
pub trait MetacallValue: MetacallClone + MetacallDowncast + Debug {
    // It tries to convert the raw pointer to the value or return a trait object on failure.
    #[doc(hidden)]
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>>
    where
        Self: Sized,
    {
        let value = Self::from_metacall_raw_leak(v)?;

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    // Same as `from_metacall_raw` but doesn't free the memory on drop and leaks.
    #[doc(hidden)]
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>>
    where
        Self: Sized;
    // It returns the enum index of Metacall Protocol in the core. It's used for faster type matching.
    #[doc(hidden)]
    fn get_metacall_id() -> u32
    where
        Self: Sized;
    // It converts the value to a raw value known by the metacall core.
    #[doc(hidden)]
    fn into_metacall_raw(self) -> *mut c_void;
}
/// Equivalent to Metacall boolean type.
impl MetacallValue for bool {
    fn get_metacall_id() -> u32 {
        0
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_bool(v) != 0 };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_bool((self as c_int).try_into().unwrap()) }
    }
}
/// Equivalent to Metacall char type.
impl MetacallValue for char {
    fn get_metacall_id() -> u32 {
        1
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_char(v) as u8 as char };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_char(self as c_char) }
    }
}
/// Equivalent to Metacall short type.
impl MetacallValue for i16 {
    fn get_metacall_id() -> u32 {
        2
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_short(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_short(self) }
    }
}
/// Equivalent to Metacall int type.
impl MetacallValue for i32 {
    fn get_metacall_id() -> u32 {
        3
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_int(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_int(self) }
    }
}
/// Equivalent to Metacall long type.
impl MetacallValue for i64 {
    fn get_metacall_id() -> u32 {
        4
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_long(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_long(self) }
    }
}
/// Equivalent to Metacall float type.
impl MetacallValue for f32 {
    fn get_metacall_id() -> u32 {
        5
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_float(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_float(self) }
    }
}
/// Equivalent to Metacall double type.
impl MetacallValue for f64 {
    fn get_metacall_id() -> u32 {
        6
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let value = unsafe { metacall_value_to_double(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_double(self) }
    }
}
/// Equivalent to Metacall string type.
impl MetacallValue for String {
    fn get_metacall_id() -> u32 {
        7
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let c_str = unsafe { CStr::from_ptr(metacall_value_to_string(v)) };
        let value = String::from(c_str.to_str().unwrap());

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        let raw = cstring!(self.as_str()).unwrap();

        unsafe { metacall_value_create_string(raw.as_ptr(), self.len()) }
    }
}
/// Equivalent to Metacall buffer type.
impl MetacallValue for Vec<i8> {
    fn get_metacall_id() -> u32 {
        8
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(unsafe {
            slice::from_raw_parts(
                metacall_value_to_buffer(v) as *mut c_char,
                metacall_value_size(v),
            )
            .to_vec()
        })
    }
    fn into_metacall_raw(mut self) -> *mut c_void {
        unsafe { metacall_value_create_buffer(self.as_mut_ptr() as *mut c_void, self.len()) }
    }
}
/// Equivalent to Metacall array type.
impl<T: MetacallValue + Clone> MetacallValue for Vec<T> {
    fn get_metacall_id() -> u32 {
        9
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        let count = unsafe { metacall_value_count(v) };
        let vec = unsafe { slice::from_raw_parts(metacall_value_to_array(v), count) }
            .iter()
            .map(|element| parsers::raw_to_metacallobj_leak(*element))
            .collect::<Result<Vec<T>, Box<dyn MetacallValue>>>()?;

        Ok(vec)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        let mut array = self
            .into_iter()
            .map(|element| element.into_metacall_raw().cast_const())
            .collect::<Vec<*const c_void>>();

        unsafe { metacall_value_create_array(array.as_mut_ptr(), array.len()) }
    }
}
/// Equivalent to Metacall map type.
impl<T: MetacallValue + Clone> MetacallValue for HashMap<String, T> {
    fn get_metacall_id() -> u32 {
        10
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        unsafe {
            let mut hashmap = HashMap::new();
            for map_value in
                slice::from_raw_parts(metacall_value_to_map(v), metacall_value_count(v)).iter()
            {
                let m_pair = slice::from_raw_parts(metacall_value_to_array(*map_value), 2);
                let key = match_metacall_value!(parsers::raw_to_metacallobj_untyped_leak::<T>(m_pair[0]), {
                    str: String => str,
                    num: i16 => num.to_string(),
                    num: i32 => num.to_string(),
                    num: i64 => num.to_string(),
                    num: f32 => num.to_string(),
                    num: f64 => num.to_string(),
                    _ =>  String::from("Invalid key!")
                });
                let val = match parsers::raw_to_metacallobj_leak::<T>(m_pair[1]) {
                    Ok(parsed) => parsed,
                    Err(original) => {
                        return Err(original);
                    }
                };

                hashmap.insert(key, val);
            }

            Ok(hashmap)
        }
    }
    fn into_metacall_raw(self) -> *mut c_void {
        let mut hashmap: Vec<*const c_void> = self
            .into_iter()
            .map(|(key, value)| {
                let key = key.into_metacall_raw();
                let value = value.into_metacall_raw();

                unsafe {
                    metacall_value_create_array(
                        vec![key as *const c_void, value as *const c_void].as_mut_ptr(),
                        2,
                    ) as *const c_void
                }
            })
            .collect();

        unsafe { metacall_value_create_map(hashmap.as_mut_ptr(), hashmap.len()) }
    }
}
/// Equivalent to Metacall pointer type.
impl MetacallValue for MetacallPointer {
    fn get_metacall_id() -> u32 {
        11
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Self::new_raw_leak(v)
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Self::new_raw(v)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall future type.
impl<T: 'static + Debug> MetacallValue for MetacallFuture<T> {
    fn get_metacall_id() -> u32 {
        12
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall function type.
impl MetacallValue for MetacallFunction {
    fn get_metacall_id() -> u32 {
        13
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall null type.
impl MetacallValue for MetacallNull {
    fn get_metacall_id() -> u32 {
        14
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        unsafe { metacall_value_destroy(v) };

        Ok(MetacallNull())
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Self::from_metacall_raw_leak(v)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall class type.
impl MetacallValue for MetacallClass {
    fn get_metacall_id() -> u32 {
        15
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall object type.
impl MetacallValue for MetacallObject {
    fn get_metacall_id() -> u32 {
        16
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall exception type.
impl MetacallValue for MetacallException {
    fn get_metacall_id() -> u32 {
        17
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to Metacall throwable type.
impl MetacallValue for MetacallThrowable {
    fn get_metacall_id() -> u32 {
        18
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
