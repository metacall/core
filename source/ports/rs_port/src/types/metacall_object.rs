use super::{
    MetacallError, MetacallGetAttributeError, MetacallNull, MetacallSetAttributeError,
    MetacallValue,
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
use super::MetacallClass;

/// Represents Metacall Object. You can get this type when returned by a function or create one from
/// a class with [create_object](MetacallClass#method.create_object).
pub struct MetacallObject {
    value: *mut c_void,
    leak: bool,
}
unsafe impl Send for MetacallObject {}
unsafe impl Sync for MetacallObject {}
impl Clone for MetacallObject {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetacallObject {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetacallObject {{ ... }}")
    }
}

impl MetacallObject {
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
    ) -> Result<*mut c_void, MetacallGetAttributeError> {
        let c_name = cstring_enum!(name, MetacallGetAttributeError)?;

        Ok(unsafe { metacall_object_get(metacall_value_to_object(self.value), c_name.as_ptr()) })
    }
    /// Gets attribute from an object without type casting([MetacallValue](MetacallValue)).
    pub fn get_attribute_untyped<T: 'static + Debug>(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetacallValue>, MetacallGetAttributeError> {
        Ok(parsers::raw_to_metacallobj_untyped::<T>(
            self.get_attribute_inner(name)?,
        ))
    }
    /// Gets attribute from an object.
    pub fn get_attribute<T: MetacallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetacallGetAttributeError> {
        match parsers::raw_to_metacallobj::<T>(self.get_attribute_inner(name)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallGetAttributeError::FailedCasting(original)),
        }
    }

    /// Sets object attribute.
    pub fn set_attribute(
        &self,
        key: impl ToString,
        value: impl MetacallValue,
    ) -> Result<(), MetacallSetAttributeError> {
        let c_key = cstring_enum!(key, MetacallSetAttributeError)?;
        let c_arg = parsers::metacallobj_to_raw(value);
        if unsafe {
            metacall_object_set(metacall_value_to_object(self.value), c_key.as_ptr(), c_arg)
        } != 0
        {
            return Err(MetacallSetAttributeError::SetAttributeFailure);
        }

        unsafe { metacall_value_destroy(c_arg) };

        Ok(())
    }

    fn call_method_inner<T: MetacallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetacallError> {
        let c_key = cstring_enum!(key, MetacallError)?;
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
    /// Calls an object method witout type casting([MetacallValue](MetacallValue)).
    pub fn call_method_untyped<T: MetacallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetacallValue>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped::<T>(
            self.call_method_inner::<T>(key, args)?,
        ))
    }
    /// Calls an object method witout type casting([MetacallValue](MetacallValue)) and
    /// without passing arguments.
    pub fn call_method_untyped_no_arg<T: MetacallValue>(
        &self,
        key: impl ToString,
    ) -> Result<Box<dyn MetacallValue>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped::<T>(
            self.call_method_inner::<T>(key, [])?,
        ))
    }
    /// Calls an object method.
    pub fn call_method<T: MetacallValue, U: MetacallValue>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_method_inner::<U>(key, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    /// Calls an object method without passing arguments.
    pub fn call_method_no_arg<T: MetacallValue>(
        &self,
        key: impl ToString,
    ) -> Result<T, MetacallError> {
        self.call_method::<T, MetacallNull>(key, [])
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

impl Drop for MetacallObject {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
