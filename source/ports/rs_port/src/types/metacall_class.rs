use super::{
    MetaCallClassFromNameError, MetaCallError, MetaCallGetAttributeError, MetaCallNull,
    MetaCallObject, MetaCallSetAttributeError, MetaCallStringConversionError, MetaCallValue,
};
use crate::{bindings::*, cast, cstring, cstring_enum};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

/// Represents MetaCall Class. You can get this type when returned by a function or get a class by its
/// name with [from_name](#method.from_name).
pub struct MetaCallClass {
    found_by_name: bool,
    leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetaCallClass {}
unsafe impl Sync for MetaCallClass {}
impl Clone for MetaCallClass {
    fn clone(&self) -> Self {
        Self {
            found_by_name: self.found_by_name,
            leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetaCallClass {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCallClass {{ ... }}")
    }
}

impl MetaCallClass {
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
    pub fn from_name(name: impl ToString) -> Result<Self, MetaCallClassFromNameError> {
        let c_name = cstring_enum!(name, MetaCallClassFromNameError)?;
        let class = unsafe { metacall_class(c_name.as_ptr()) };

        if class.is_null() {
            return Err(MetaCallClassFromNameError::ClassNotFound);
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

    /// Creates an [object](MetaCallObject) of the class wtih constructor arguments.
    pub fn create_object<T: MetaCallValue>(
        &self,
        name: impl ToString,
        constructor_args: impl IntoIterator<Item = T>,
    ) -> Result<MetaCallObject, MetaCallStringConversionError> {
        let c_name = cstring!(name)?;
        let mut c_args = cast::metacallobj_to_raw_args(constructor_args);
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

        Ok(MetaCallObject::new_raw(obj))
    }
    /// Creates an [object](MetaCallObject) of the class wtihout constructor arguments.
    pub fn create_object_no_arg(
        &self,
        name: impl ToString,
    ) -> Result<MetaCallObject, MetaCallStringConversionError> {
        self.create_object::<MetaCallNull>(name, [])
    }

    fn get_attribute_inner(
        &self,
        name: impl ToString,
    ) -> Result<*mut c_void, MetaCallGetAttributeError> {
        let c_name = cstring_enum!(name, MetaCallGetAttributeError)?;

        Ok(unsafe { metacall_class_static_get(self.value_to_class(), c_name.as_ptr()) })
    }
    /// Gets static attribute from a class without type casting([MetaCallValue](MetaCallValue)).
    pub fn get_attribute_untyped(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallGetAttributeError> {
        Ok(cast::raw_to_metacallobj_untyped(
            self.get_attribute_inner(name)?,
        ))
    }
    /// Gets static attribute from a class.
    pub fn get_attribute<T: MetaCallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetaCallGetAttributeError> {
        match cast::raw_to_metacallobj::<T>(self.get_attribute_inner(name)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetaCallGetAttributeError::FailedCasting(original)),
        }
    }

    /// Sets static class attribute.
    pub fn set_attribute(
        &self,
        key: impl ToString,
        value: impl MetaCallValue,
    ) -> Result<(), MetaCallSetAttributeError> {
        let c_key = cstring_enum!(key, MetaCallSetAttributeError)?;

        let c_arg = cast::metacallobj_to_raw(value);
        if unsafe { metacall_class_static_set(self.value_to_class(), c_key.as_ptr(), c_arg) } != 0 {
            return Err(MetaCallSetAttributeError::SetAttributeFailure);
        }

        unsafe { metacall_value_destroy(c_arg) };

        Ok(())
    }

    fn call_method_inner<T: MetaCallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetaCallError> {
        let c_key = cstring_enum!(name, MetaCallError)?;
        let mut c_args = cast::metacallobj_to_raw_args(args);
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
    /// Calls a static class method witout type casting([MetaCallValue](MetaCallValue)).
    pub fn call_method_untyped<T: MetaCallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
        Ok(cast::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(name, args)?,
        ))
    }
    /// Calls a static class method witout type casting([MetaCallValue](MetaCallValue)) and
    /// without passing arguments.
    pub fn call_method_untyped_no_arg<T: MetaCallValue>(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetaCallValue>, MetaCallError> {
        Ok(cast::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(name, [])?,
        ))
    }
    /// Calls a static class method.
    pub fn call_method<T: MetaCallValue, U: MetaCallValue>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetaCallError> {
        match cast::raw_to_metacallobj::<T>(self.call_method_inner::<U>(name, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetaCallError::FailedCasting(original)),
        }
    }
    /// Calls a static class method without passing arguments.
    pub fn call_method_no_arg<T: MetaCallValue>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetaCallError> {
        self.call_method::<T, MetaCallNull>(name, [])
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;

        self.value
    }
}

impl Drop for MetaCallClass {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }
    }
}
