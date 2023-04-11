use super::{
    MetacallClass, MetacallException, MetacallFunction, MetacallFuture, MetacallNull,
    MetacallObject, MetacallPointer, MetacallThrowable,
};
use crate::{bindings::*, cstring, match_object_protocol, match_object_protocol_all, parsers};
use dyn_clone::DynClone;
use std::{
    any::Any,
    collections::HashMap,
    ffi::{c_char, c_int, c_void, CStr, CString},
    fmt::Debug,
    slice,
};

pub trait Downcast: Any {
    fn into_any(self: Box<Self>) -> Box<dyn Any>;
    fn as_any(&self) -> &dyn Any;
    fn as_any_mut(&mut self) -> &mut dyn Any;
}
impl<T: Any> Downcast for T {
    fn into_any(self: Box<Self>) -> Box<dyn Any> {
        self
    }
    fn as_any(&self) -> &dyn Any {
        self
    }
    fn as_any_mut(&mut self) -> &mut dyn Any {
        self
    }
}

pub trait MetacallObjectProtocol: Downcast + Debug + DynClone {
    // It tries to convert the raw pointer to the value and return a trait object on failure
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>>
    where
        Self: Sized;
    // It returns the enum index of Metacall Protocol in the core
    fn get_metacall_id() -> u32
    where
        Self: Sized;
    // It returns a tuple. The first element is a raw pointer to the value parsed with metacall_value_to_X
    // and the second element is a closure that drops the pointer (if it exists depending on the type)
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>);
}
dyn_clone::clone_trait_object!(MetacallObjectProtocol);

impl dyn MetacallObjectProtocol {
    #[inline]
    pub fn is<T: MetacallObjectProtocol>(&self) -> bool {
        Downcast::as_any(self).is::<T>()
    }

    #[inline]
    pub fn downcast<T: MetacallObjectProtocol>(self: Box<Self>) -> Result<T, Box<Self>> {
        if self.is::<T>() {
            Ok(*Downcast::into_any(self).downcast::<T>().unwrap())
        } else {
            Err(self)
        }
    }

    #[inline]
    pub fn downcast_ref<T: MetacallObjectProtocol>(&self) -> Option<&T> {
        Downcast::as_any(self).downcast_ref::<T>()
    }

    #[inline]
    pub fn downcast_mut<T: MetacallObjectProtocol>(&mut self) -> Option<&mut T> {
        Downcast::as_any_mut(self).downcast_mut::<T>()
    }
}

impl MetacallObjectProtocol for bool {
    fn get_metacall_id() -> u32 {
        0
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_bool(v) != 0 };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_bool((self as c_int).try_into().unwrap()) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr);
            })),
        )
    }
}
impl MetacallObjectProtocol for char {
    fn get_metacall_id() -> u32 {
        1
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_char(v) as u8 as char };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_char(self as c_char) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for i16 {
    fn get_metacall_id() -> u32 {
        2
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_short(v) };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_short(self) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for i32 {
    fn get_metacall_id() -> u32 {
        3
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_int(v) };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_int(self) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for i64 {
    fn get_metacall_id() -> u32 {
        4
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_long(v) };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_long(self) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for f32 {
    fn get_metacall_id() -> u32 {
        5
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_float(v) };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_float(self) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for f64 {
    fn get_metacall_id() -> u32 {
        6
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let value = unsafe { metacall_value_to_double(v) };

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let value_ptr = unsafe { metacall_value_create_double(self) };

        (
            value_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(value_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for String {
    fn get_metacall_id() -> u32 {
        7
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let c_str = unsafe { CStr::from_ptr(metacall_value_to_string(v)) };
        let value = String::from(c_str.to_str().unwrap());

        unsafe { metacall_value_destroy(v) };

        Ok(value)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let raw = cstring!(self.as_str()).unwrap().into_raw();
        let new_raw = unsafe { metacall_value_create_string(raw.cast(), self.len()) };

        (
            new_raw,
            Some(Box::new(move || unsafe {
                drop(CString::from_raw(raw));

                metacall_value_destroy(new_raw);
            })),
        )
    }
}
impl MetacallObjectProtocol for Vec<i8> {
    fn get_metacall_id() -> u32 {
        8
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        unsafe {
            let buf = slice::from_raw_parts(
                metacall_value_to_buffer(v) as *mut c_char,
                metacall_value_size(v),
            )
            .to_vec();

            metacall_value_destroy(v);

            Ok(buf)
        }
    }
    fn into_metacall_raw(mut self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let new_raw =
            unsafe { metacall_value_create_buffer(self.as_mut_ptr() as *mut c_void, self.len()) };
        let raw = self.as_mut_ptr();

        (
            new_raw,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(new_raw);

                raw.drop_in_place();
            })),
        )
    }
}
impl<T: MetacallObjectProtocol + Clone> MetacallObjectProtocol for Vec<T> {
    fn get_metacall_id() -> u32 {
        9
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let count = unsafe { metacall_value_count(v) };
        let array = unsafe { metacall_value_to_array(v) };
        let vec = unsafe { slice::from_raw_parts_mut(array, count).to_vec() };
        let vec = vec
            .into_iter()
            .map(|element| parsers::raw_to_metacallobj(element))
            .collect::<Result<Vec<T>, Box<dyn MetacallObjectProtocol>>>()?;

        // unsafe { metacall_value_destroy() };

        Ok(vec)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let mut value_drops = Vec::with_capacity(self.capacity());
        let mut array = self
            .into_iter()
            .map(|element| {
                let (element, value_drop) = element.into_metacall_raw();

                if let Some(value_drop) = value_drop {
                    value_drops.push(value_drop);
                }

                element as *const c_void
            })
            .collect::<Vec<*const c_void>>();

        let raw = array.as_mut_ptr();
        let new_raw = unsafe { metacall_value_create_array(raw, array.len()) };

        (
            new_raw,
            Some(Box::new(move || unsafe {
                for value_drop in value_drops {
                    value_drop();
                }
                // raw.drop_in_place();

                metacall_value_destroy(new_raw);
            })),
        )
    }
}
impl<T: MetacallObjectProtocol + Clone> MetacallObjectProtocol for HashMap<String, T> {
    fn get_metacall_id() -> u32 {
        10
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        unsafe {
            let map = metacall_value_to_map(v);
            let count = metacall_value_count(v);
            let map = slice::from_raw_parts(map, count);

            let mut hashmap = HashMap::new();
            for map_value in map.iter() {
                let m_pair = metacall_value_to_array(*map_value);
                let m_pair = slice::from_raw_parts(m_pair, 2);

                let key = parsers::hashmap_key_with_downcast(parsers::raw_to_metacallobj_untyped(
                    m_pair[0],
                ));
                let val = match parsers::raw_to_metacallobj::<T>(m_pair[1]) {
                    Ok(parsed) => parsed,
                    Err(original) => {
                        return Err(original);
                    }
                };

                hashmap.insert(key, val);
            }

            metacall_value_destroy(v);

            Ok(hashmap)
        }
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let mut value_drops = Vec::with_capacity(self.len());
        let mut hashmap: Vec<*const c_void> = self
            .into_iter()
            .map(|(key, value)| {
                let (key, value_drop1) = key.into_metacall_raw();
                let (value, value_drop2) = value.into_metacall_raw();

                if let Some(value_drop) = value_drop1 {
                    value_drops.push(value_drop);
                }
                if let Some(value_drop) = value_drop2 {
                    value_drops.push(value_drop);
                }

                parsers::new_void_pointer((key, value)) as *const c_void
            })
            .collect();

        let new_raw = unsafe { metacall_value_create_map(hashmap.as_mut_ptr(), hashmap.len()) };
        let raw = hashmap.as_mut_ptr();

        (
            new_raw,
            Some(Box::new(move || unsafe {
                for value_drop in value_drops {
                    value_drop();
                }

                metacall_value_destroy(new_raw);

                raw.drop_in_place()
            })),
        )
    }
}
impl MetacallObjectProtocol for MetacallPointer {
    fn get_metacall_id() -> u32 {
        11
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Self::new_raw(v)
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallFuture {
    fn get_metacall_id() -> u32 {
        12
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallFuture::new(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallFunction {
    fn get_metacall_id() -> u32 {
        13
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallFunction::new_raw(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallNull {
    fn get_metacall_id() -> u32 {
        14
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        if !v.is_null() {
            unsafe { metacall_value_destroy(v) };
        }

        Ok(MetacallNull())
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        let null_ptr = self.into_raw();

        (
            null_ptr,
            Some(Box::new(move || unsafe {
                metacall_value_destroy(null_ptr)
            })),
        )
    }
}
impl MetacallObjectProtocol for MetacallClass {
    fn get_metacall_id() -> u32 {
        15
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallClass::from(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallObject {
    fn get_metacall_id() -> u32 {
        16
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallObject::new(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallException {
    fn get_metacall_id() -> u32 {
        17
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallException::from(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for MetacallThrowable {
    fn get_metacall_id() -> u32 {
        18
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(MetacallThrowable::new(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        (self.into_raw(), None)
    }
}
impl MetacallObjectProtocol for Box<dyn MetacallObjectProtocol> {
    fn get_metacall_id() -> u32 {
        // Something random
        100
    }
    fn from_metacall_raw(v: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(parsers::raw_to_metacallobj_untyped(v))
    }
    fn into_metacall_raw(self) -> (*mut c_void, Option<Box<dyn FnOnce()>>) {
        match_object_protocol_all!(
            self,
            x,
            x.into_metacall_raw(),
            bool,
            char,
            i16,
            i32,
            i64,
            f32,
            f64,
            String,
            Vec<i8>,
            Vec<Box<dyn MetacallObjectProtocol>>,
            HashMap<String, Box<dyn MetacallObjectProtocol>>,
            MetacallPointer,
            MetacallFuture,
            MetacallFunction,
            MetacallNull,
            MetacallClass,
            MetacallObject,
            MetacallException,
            MetacallThrowable,
            Box<dyn MetacallObjectProtocol>
        )
    }
}
