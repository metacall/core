use super::{
    MetaCallClass, MetaCallException, MetaCallFunction, MetaCallFuture, MetaCallNull,
    MetaCallObject, MetaCallPointer, MetaCallThrowable,
};
use crate::{
    bindings::*,
    cast, cstring,
    helpers::{MetaCallClone, MetaCallDowncast},
    match_metacall_value,
};
use std::{
    collections::HashMap,
    ffi::{c_char, c_int, c_void, CStr},
    fmt::Debug,
    slice,
};

/// Trait of any possible object in MetaCall.
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
/// metacall::metacall_untyped("x", [metacall::MetaCallPointer::new(String::from("hi"))]);
/// // future?
/// // nope! you can't pass a future!
/// // function
/// metacall::metacall_untyped("x", [
///     metacall::metacall_no_arg::<metacall::MetaCallFunction>("my_async_function").unwrap()
/// ]);
/// // null
/// metacall::metacall_untyped("x", [metacall::MetaCallNull()]);
/// // class
/// metacall::metacall_untyped("x", [metacall::MetaCallClass::from_name("MyClass").unwrap()]);
/// // object
/// let class = metacall::MetaCallClass::from_name("MyClass").unwrap();
/// metacall::metacall_untyped("x", [class.create_object_no_arg("myObject").unwrap()]);
/// // exception
/// metacall::metacall_untyped("x", [
///     metacall::metacall_no_arg::<metacall::MetaCallException>("my_function").unwrap()
/// ]);
/// // throwable?
/// // nope! you can't pass a throwable!
/// ```
pub trait MetaCallValue: MetaCallClone + MetaCallDowncast + Debug {
    // It tries to convert the raw pointer to the value or return a trait object on failure.
    #[doc(hidden)]
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>>
    where
        Self: Sized,
    {
        let value = Self::from_metacall_raw_leak(v)?;

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    // Same as `from_metacall_raw` but doesn't free the memory on drop and leaks.
    #[doc(hidden)]
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>>
    where
        Self: Sized;
    // It returns the enum index of MetaCall Protocol in the core. It's used for faster type matching.
    #[doc(hidden)]
    fn get_metacall_id() -> metacall_value_id
    where
        Self: Sized;
    // It converts the value to a raw value known by the metacall core.
    #[doc(hidden)]
    fn into_metacall_raw(self) -> *mut c_void;
}
/// Equivalent to MetaCall boolean type.
impl MetaCallValue for bool {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_BOOL
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_bool(v) != 0 };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_bool((self as c_int).try_into().unwrap()) }
    }
}
/// Equivalent to MetaCall char type.
impl MetaCallValue for char {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_CHAR
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_char(v) as u8 as char };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_char(self as c_char) }
    }
}
/// Equivalent to MetaCall short type.
impl MetaCallValue for i16 {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_SHORT
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_short(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_short(self) }
    }
}
/// Equivalent to MetaCall int type.
impl MetaCallValue for i32 {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_INT
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_int(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_int(self) }
    }
}
/// Equivalent to MetaCall long type.
impl MetaCallValue for i64 {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_LONG
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_long(v) };

        Ok(value as i64)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_long(self.try_into().unwrap()) }
    }
}
/// Equivalent to MetaCall float type.
impl MetaCallValue for f32 {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_FLOAT
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_float(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_float(self) }
    }
}
/// Equivalent to MetaCall double type.
impl MetaCallValue for f64 {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_DOUBLE
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let value = unsafe { metacall_value_to_double(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_double(self) }
    }
}
/// Equivalent to MetaCall string type.
impl MetaCallValue for String {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_STRING
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let c_str = unsafe { CStr::from_ptr(metacall_value_to_string(v)) };
        let value = String::from(c_str.to_str().unwrap());

        Ok(value)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        let raw = cstring!(self.as_str()).unwrap();

        unsafe { metacall_value_create_string(raw.as_ptr(), self.len()) }
    }
}
/// Equivalent to MetaCall buffer type.
impl MetaCallValue for Vec<i8> {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_BUFFER
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
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
/// Equivalent to MetaCall array type.
impl<T: MetaCallValue + Clone> MetaCallValue for Vec<T> {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_ARRAY
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        let count = unsafe { metacall_value_count(v) };
        let vec = unsafe { slice::from_raw_parts(metacall_value_to_array(v), count) }
            .iter()
            .map(|element| cast::raw_to_metacallobj_leak(*element))
            .collect::<Result<Vec<T>, Box<dyn MetaCallValue>>>()?;

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
/// Equivalent to MetaCall map type.
impl<T: MetaCallValue + Clone> MetaCallValue for HashMap<String, T> {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_MAP
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        unsafe {
            let mut hashmap = HashMap::new();
            for map_value in
                slice::from_raw_parts(metacall_value_to_map(v), metacall_value_count(v)).iter()
            {
                let m_pair = slice::from_raw_parts(metacall_value_to_array(*map_value), 2);
                let key = match_metacall_value!(cast::raw_to_metacallobj_untyped_leak(m_pair[0]), {
                    str: String => str,
                    num: i16 => num.to_string(),
                    num: i32 => num.to_string(),
                    num: i64 => num.to_string(),
                    num: f32 => num.to_string(),
                    num: f64 => num.to_string(),
                    _ =>  String::from("Invalid key!")
                });
                let val = match cast::raw_to_metacallobj_leak::<T>(m_pair[1]) {
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
/// Equivalent to MetaCall pointer type.
impl MetaCallValue for MetaCallPointer {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_PTR
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Self::new_raw_leak(v)
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Self::new_raw(v)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall future type.
impl MetaCallValue for MetaCallFuture {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_FUTURE
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall function type.
impl MetaCallValue for MetaCallFunction {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_FUNCTION
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall null type.
impl MetaCallValue for MetaCallNull {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_NULL
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        unsafe { metacall_value_destroy(v) };

        Ok(MetaCallNull())
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Self::from_metacall_raw_leak(v)
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall class type.
impl MetaCallValue for MetaCallClass {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_CLASS
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall object type.
impl MetaCallValue for MetaCallObject {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_OBJECT
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall exception type.
impl MetaCallValue for MetaCallException {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_EXCEPTION
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Equivalent to MetaCall throwable type.
impl MetaCallValue for MetaCallThrowable {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_THROWABLE
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw_leak(v))
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self::new_raw(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        self.into_raw()
    }
}
/// Just a Rust barrier made for easier polymorphism.
impl MetaCallValue for Box<dyn MetaCallValue> {
    fn get_metacall_id() -> metacall_value_id {
        metacall_value_id::METACALL_INVALID
    }
    fn from_metacall_raw_leak(v: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(cast::raw_to_metacallobj_untyped_leak(v))
    }
    fn into_metacall_raw(self) -> *mut c_void {
        match_metacall_value!(self, {
            bool: bool => bool.into_metacall_raw(),
            char: char => char.into_metacall_raw(),
            num: i16 => num.into_metacall_raw(),
            num: i32 => num.into_metacall_raw(),
            num: i64 => num.into_metacall_raw(),
            num: f32 => num.into_metacall_raw(),
            num: f64 => num.into_metacall_raw(),
            str: String => str.into_metacall_raw(),
            buf: Vec<i8> => buf.into_metacall_raw(),
            arr: Vec<Box<dyn MetaCallValue>> => arr.into_metacall_raw(),
            map: HashMap<String, Box<dyn MetaCallValue>> => map.into_metacall_raw(),
            ptr: MetaCallPointer => ptr.into_metacall_raw(),
            fut: MetaCallFuture => fut.into_metacall_raw(),
            fun: MetaCallFunction => fun.into_metacall_raw(),
            null: MetaCallNull => null.into_metacall_raw(),
            cls: MetaCallClass => cls.into_metacall_raw(),
            obj: MetaCallObject => obj.into_metacall_raw(),
            exc: MetaCallException => exc.into_metacall_raw(),
            thr: MetaCallThrowable => thr.into_metacall_raw(),
            _ => MetaCallNull().into_metacall_raw()
        })
    }
}
