use super::{
    MetacallError, MetacallGetAttributeError, MetacallNull, MetacallObjectProtocol,
    MetacallSetAttributeError,
};
use crate::{
    bindings::{
        metacall_object_get, metacall_object_set, metacall_value_destroy, metacall_value_to_object,
        metacallv_object,
    },
    cstring_enum, parsers,
};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Debug)]
pub struct MetacallObject {
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallObject {}
unsafe impl Sync for MetacallObject {}

impl MetacallObject {
    pub fn new(ptr: *mut c_void) -> Self {
        Self { ptr: Arc::new(ptr) }
    }

    fn value_to_object(&self) -> *mut c_void {
        unsafe { metacall_value_to_object(*self.ptr) }
    }

    fn get_attribute_inner(
        &self,
        name: impl ToString,
    ) -> Result<*mut c_void, MetacallGetAttributeError> {
        let c_name = cstring_enum!(name, MetacallGetAttributeError)?;

        Ok(unsafe { metacall_object_get(self.value_to_object(), c_name.as_ptr()) })
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
        if unsafe { metacall_object_set(self.value_to_object(), c_key.as_ptr(), c_args) } != 0 {
            return Err(MetacallSetAttributeError::SetAttributeFailure);
        }

        cleanup();

        Ok(())
    }

    fn call_method_inner<T: MetacallObjectProtocol>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<*mut c_void, MetacallError> {
        let c_key = cstring_enum!(key, MetacallError)?;
        let (mut args, cleanup) = parsers::metacallobj_to_raw_args(args);
        let ret = unsafe {
            metacallv_object(
                self.value_to_object(),
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
        key: impl ToString,
        args: impl IntoIterator<Item = T>,
    ) -> Result<Box<dyn MetacallObjectProtocol>, MetacallError> {
        Ok(parsers::raw_to_metacallobj_untyped(
            self.call_method_inner::<T>(key, args)?,
        ))
    }
    pub fn call_method<T: MetacallObjectProtocol, U: MetacallObjectProtocol>(
        &self,
        key: impl ToString,
        args: impl IntoIterator<Item = U>,
    ) -> Result<T, MetacallError> {
        match parsers::raw_to_metacallobj::<T>(self.call_method_inner::<U>(key, args)?) {
            Ok(ret) => Ok(ret),
            Err(original) => Err(MetacallError::FailedCasting(original)),
        }
    }
    pub fn call_method_no_arg<T: MetacallObjectProtocol>(
        &self,
        key: impl ToString,
    ) -> Result<T, MetacallError> {
        self.call_method::<T, MetacallNull>(key, [])
    }

    pub fn into_raw(self) -> *mut c_void {
        self.value_to_object()
    }
}

impl Drop for MetacallObject {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) }
    }
}
