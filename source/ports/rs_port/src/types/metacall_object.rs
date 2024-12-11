use super::{
    MetaCallError, MetaCallGetAttributeError, MetaCallNull, MetaCallSetAttributeError,
    MetaCallValue,
};
use crate::{
    bindings::{
        metacall_object_get, metacall_object_set, metacall_value_destroy, metacall_value_to_object,
        metacallv_object,
    },
    cstring_enum, parsers,
};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

// Used for documentation.
#[allow(unused_imports)]
use super::MetaCallClass;

/// Represents MetaCall Object. You can get this type when returned by a function or create one from
/// a class with [create_object](MetaCallClass#method.create_object).
pub struct MetaCallObject {
    value: *mut c_void,
    leak: bool,
}
unsafe impl Send for MetaCallObject {}
unsafe impl Sync for MetaCallObject {}
impl Clone for MetaCallObject {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetaCallObject {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCallObject {{ ... }}")
    }
}

impl MetaCallObject {
    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        Self { value, leak: false }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        Self { value, leak: true }
    }

    fn get_attribute_inner(
        &self,
        name: impl ToString,
    ) -> Result<*mut c_void, MetaCallGetAttributeError> {
        let c_name = cstring_enum!(name, MetaCallGetAttributeError)?;

        Ok(unsafe { metacall_object_get(metacall_value_to_object(self.value), c_name.as_ptr()) })
    }
    /// Gets attribute from an object without type casting([MetaCallValue](MetaCallValue)).
    pub fn get_attribute_untyped(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallGetAttributeError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.get_attribute_inner(name)?,
        ))
    }
    /// Gets attribute from an object.
    pub fn get_attribute<T: MetaCallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetaCallGetAttributeError> {
        match parsers::raw_to_metacallobj::<T>(self.get_attribute_inner(name)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetaCallGetAttributeError::FailedCasting(original)),
        }
    }

    /// Sets object attribute.
    pub fn set_attribute(
        &self,
        key: impl ToString,
        value: impl MetaCallValue,
    ) -> Result<(), MetaCallSetAttributeError> {
        let c_key = cstring_enum!(key, MetaCallSetAttributeError)?;
        let c_arg = parsers::metacallobj_to_raw(value);
        if unsafe {
            metacall_object_set(metacall_value_to_object(self.value), c_key.as_ptr(), c_arg)
        } != 0
        {
            return Err(MetaCallSetAttributeError::SetAttributeFailure);
        }

        unsafe { metacall_value_destroy(c_arg) };

        Ok(())
    }

    fn call_method_inner<T: MetaCallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetaCallError> {
        let c_key = cstring_enum!(key, MetaCallError)?;
        let mut c_args = parsers::metacallobj_to_raw_args(args);
        let ret = unsafe {
            metacallv_object(
                metacall_value_to_object(self.value),
                c_key.as_ptr(),
                c_args.as_mut_ptr(),
                c_args.len(),
            )
        };

        for c_arg in c_args {
            unsafe { metacall_value_destroy(c_arg) };
        }

        Ok(ret)
    }
    /// Calls an object method witout type casting([MetaCallValue](MetaCallValue)).
    pub fn call_method_untyped<T: MetaCallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(key, args)?,
        ))
    }
    /// Calls an object method witout type casting([MetaCallValue](MetaCallValue)) and
    /// without passing arguments.
    pub fn call_method_untyped_no_arg<T: MetaCallValue>(
        &self,
        key: impl ToString,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(key, [])?,
        ))
    }
    /// Calls an object method.
    pub fn call_method<T: MetaCallValue, U: MetaCallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetaCallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_method_inner::<U>(key, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetaCallError::FailedCasting(original)),
        }
    }
    /// Calls an object method without passing arguments.
    pub fn call_method_no_arg<T: MetaCallValue>(
        &self,
        key: impl ToString,
    ) -> Result<T, MetaCallError> {
        self.call_method::<T, MetaCallNull>(key, [])
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

impl Drop for MetaCallObject {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
