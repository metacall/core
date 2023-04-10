use super::{
    MetacallError, MetacallGetAttributeError, MetacallNull, MetacallObject, MetacallObjectProtocol,
    MetacallSetAttributeError, MetacallStringConversionError,
};
use crate::{bindings::*, cstring, cstring_enum, parsers};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Debug)]
pub struct MetacallClass {
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallClass {}
unsafe impl Sync for MetacallClass {}

impl MetacallClass {
    pub fn from(ptr: *mut c_void) -> Self {
        Self { ptr: Arc::new(ptr) }
    }

    pub fn from_name(name: impl ToString) -> Result<Self, MetacallStringConversionError> {
        let c_name = cstring!(name)?;

        Ok(Self {
            ptr: Arc::new(unsafe { metacall_class(c_name.as_ptr()) }),
        })
    }

    fn value_to_class(&self) -> *mut c_void {
        unsafe { metacall_value_to_class(*self.ptr) }
    }

    pub fn create_object<T: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
        constructor_args: impl IntoIterator<Item = T>,
    ) -> Result<MetacallObject, MetacallStringConversionError> {
        let c_name = cstring!(name)?;
        let (mut c_args, cleanup) = parsers::metacallobj_to_raw_args(constructor_args);
        let obj = unsafe {
            metacall_class_new(
                self.value_to_class(),
                c_name.as_ptr(),
                c_args.as_mut_ptr(),
                c_args.len(),
            )
        };

        cleanup();

        Ok(MetacallObject::new(obj))
    }
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
    pub fn get_attribute_untyped(
        &self,
        name: impl ToString,
    ) -> Result<Box<dyn MetacallObjectProtocol>, MetacallGetAttributeError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.get_attribute_inner(name)?,
        ))
    }
    pub fn get_attribute<T: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetacallGetAttributeError> {
        match parsers::raw_to_metacallobj::<T>(self.get_attribute_inner(name)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallGetAttributeError::FailedCasting(original)),
        }
    }

    pub fn set_attribute(
        &self,
        key: impl ToString,
        value: impl MetacallObjectProtocol,
    ) -> Result<(), MetacallSetAttributeError> {
        let c_key = cstring_enum!(key, MetacallSetAttributeError)?;

        let (c_args, cleanup) = parsers::metacallobj_to_raw(value);
        if unsafe { metacall_class_static_set(self.value_to_class(), c_key.as_ptr(), c_args) } != 0
        {
            return Err(MetacallSetAttributeError::SetAttributeFailure);
        }

        cleanup();

        Ok(())
    }

    fn call_method_inner<T: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetacallError> {
        let c_key = cstring_enum!(name, MetacallError)?;
        let (mut args, cleanup) = parsers::metacallobj_to_raw_args(args);
        let ret = unsafe {
            metacallv_class(
                self.value_to_class(),
                c_key.as_ptr(),
                args.as_mut_ptr(),
                args.len(),
            )
        };

        cleanup();

        Ok(ret)
    }
    pub fn call_method_untyped<T: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetacallObjectProtocol>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(name, args)?,
        ))
    }
    pub fn call_method<T: MetacallObjectProtocol, U: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_method_inner::<U>(name, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    pub fn call_method_no_arg<T: MetacallObjectProtocol>(
        &self,
        name: impl ToString,
    ) -> Result<T, MetacallError> {
        self.call_method::<T, MetacallNull>(name, [])
    }

    pub fn into_raw(self) -> *mut c_void {
        self.value_to_class()
    }
}

impl Drop for MetacallClass {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) }
    }
}
