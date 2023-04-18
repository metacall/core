use super::{
    MetacallClassFromNameError, MetacallError, MetacallGetAttributeError, MetacallNull,
    MetacallObject, MetacallSetAttributeError, MetacallStringConversionError, MetacallValue,
};
use crate::{bindings::*, cstring, cstring_enum, parsers};
use std::ffi::c_void;

#[derive(Debug)]
/// Represents Metacall Class. You can get this type when returned by a function or get a class by its
/// name with [from_name](#method.from_name).
pub struct MetacallClass {
    found_by_name: bool,
    leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetacallClass {}
unsafe impl Sync for MetacallClass {}
impl Clone for MetacallClass {
    fn clone(&self) -> Self {
        Self {
            found_by_name: self.found_by_name,
            leak: true,
            value: unsafe { metacall_value_copy(self.value) },
        }
    }
}

impl MetacallClass {
    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Self {
        Self {
            found_by_name: false,
            leak: false,
            value,
        }
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Self {
        Self {
            found_by_name: false,
            leak: true,
            value,
        }
    }

    /// Gets a class by its name.
    pub fn from_name(name: impl ToString) -> Result<Self, MetacallClassFromNameError> {
        let c_name = cstring_enum!(name, MetacallClassFromNameError)?;
        let class = unsafe { metacall_class(c_name.as_ptr()) };

        if class.is_null() {
            return Err(MetacallClassFromNameError::ClassNotFound);
        }

        Ok(Self {
            found_by_name: true,
            leak: true,
            value: class,
        })
    }

    fn value_to_class(&self) -> *mut c_void {
        if self.found_by_name {
            self.value
        } else {
            unsafe { metacall_value_to_class(self.value) }
        }
    }

    /// Creates an [object](MetacallObject) of the class wtih constructor arguments.
    pub fn create_object<T: MetacallValue>(
        &self,
        name: impl ToString,
        constructor_args: impl IntoIterator<Item = T>,
    ) -> Result<MetacallObject, MetacallStringConversionError> {
        let c_name = cstring!(name)?;
        let mut c_args = parsers::metacallobj_to_raw_args(constructor_args);
        let obj = unsafe {
            metacall_class_new(
                self.value_to_class(),
                c_name.as_ptr(),
                c_args.as_mut_ptr(),
                c_args.len(),
            )
        };

        for c_arg in c_args {
            unsafe { metacall_value_destroy(c_arg) };
        }

        Ok(MetacallObject::new_raw(obj))
    }
    /// Creates an [object](MetacallObject) of the class wtihout constructor arguments.
    pub fn create_object_no_arg(
        &self,
        name: impl ToString,
    ) -> Result<MetacallObject, MetacallStringConversionError> {
        self.create_object::<MetacallNull>(name, [])
    }

    fn get_attribute_inner(
        &self,
        name: impl ToString,
    ) -> Result<*mut c_void, MetacallGetAttributeError> {
        let c_name = cstring_enum!(name, MetacallGetAttributeError)?;

        Ok(unsafe { metacall_class_static_get(self.value_to_class(), c_name.as_ptr()) })
    }
    /// Gets static attribute from a class without type casting([MetacallValue](MetacallValue)).
    pub fn get_attribute_untyped(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetacallValue>, MetacallGetAttributeError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.get_attribute_inner(name)?,
        ))
    }
    /// Gets static attribute from a class.
    pub fn get_attribute<T: MetacallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetacallGetAttributeError> {
        match parsers::raw_to_metacallobj::<T>(self.get_attribute_inner(name)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallGetAttributeError::FailedCasting(original)),
        }
    }

    /// Sets static class attribute.
    pub fn set_attribute(
        &self,
        key: impl ToString,
        value: impl MetacallValue,
    ) -> Result<(), MetacallSetAttributeError> {
        let c_key = cstring_enum!(key, MetacallSetAttributeError)?;

        let c_arg = parsers::metacallobj_to_raw(value);
        if unsafe { metacall_class_static_set(self.value_to_class(), c_key.as_ptr(), c_arg) } != 0 {
            return Err(MetacallSetAttributeError::SetAttributeFailure);
        }

        unsafe { metacall_value_destroy(c_arg) };

        Ok(())
    }

    fn call_method_inner<T: MetacallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetacallError> {
        let c_key = cstring_enum!(name, MetacallError)?;
        let mut c_args = parsers::metacallobj_to_raw_args(args);
        let ret = unsafe {
            metacallv_class(
                self.value_to_class(),
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
    /// Calls a static class method witout type casting([MetacallValue](MetacallValue)).
    pub fn call_method_untyped<T: MetacallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetacallValue>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(name, args)?,
        ))
    }
    /// Calls a static class method witout type casting([MetacallValue](MetacallValue)) and
    /// without passing arguments.
    pub fn call_method_untyped_no_arg<T: MetacallValue>(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetacallValue>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(name, [])?,
        ))
    }
    /// Calls a static class method.
    pub fn call_method<T: MetacallValue, U: MetacallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_method_inner::<U>(name, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    /// Calls a static class method without passing arguments.
    pub fn call_method_no_arg<T: MetacallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetacallError> {
        self.call_method::<T, MetacallNull>(name, [])
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

impl Drop for MetacallClass {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
