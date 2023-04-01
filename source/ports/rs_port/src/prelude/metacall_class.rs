use super::{MetacallObject, MetacallSetAttributeError, MetacallStringConversionError};
use crate::{
    bindings::*,
    cstring, cstring_enum,
    parsers::{self, any_to_metacall},
    prelude::Any,
};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Hash)]
pub struct MetacallClass {
    class: Arc<*mut c_void>,
}
unsafe impl Send for MetacallClass {}
unsafe impl Sync for MetacallClass {}

impl MetacallClass {
    pub fn from(class: *mut c_void) -> Self {
        Self {
            class: Arc::new(class),
        }
    }

    pub fn create_object(
        &self,
        name: impl ToString,
        constructor_args: impl IntoIterator<Item = Any>,
    ) -> Result<MetacallObject, MetacallStringConversionError> {
        let class = *self.class.clone();
        let c_name = cstring!(name)?;
        let mut c_args = any_to_metacall(constructor_args);

        let obj = unsafe {
            metacall_value_to_object(metacall_class_new(
                class,
                c_name.as_ptr(),
                c_args.as_mut_ptr(),
                c_args.len(),
            ))
        };

        Ok(MetacallObject::new(obj))
    }

    pub fn get_attribute<T: ToString>(
        &self,
        name: T,
    ) -> Result<Any, MetacallStringConversionError> {
        let c_name = cstring!(name)?;

        Ok(parsers::metacall_to_any(unsafe {
            metacall_class_static_get(*self.class.clone(), c_name.as_ptr())
        }))
    }

    pub fn set_attribute<T: ToString>(
        &self,
        key: T,
        value: Any,
    ) -> Result<(), MetacallSetAttributeError> {
        let c_key = cstring_enum!(key, MetacallSetAttributeError)?;

        let c_args = parsers::any_to_metacall([value])[0];
        if unsafe { metacall_class_static_set(*self.class.clone(), c_key.as_ptr(), c_args) } != 0 {
            return Err(MetacallSetAttributeError::SetAttributeFailure);
        }

        Ok(())
    }

    pub fn call_method<T: ToString>(
        &self,
        key: T,
        args: impl IntoIterator<Item = Any>,
    ) -> Result<Any, MetacallStringConversionError> {
        let c_key = cstring!(key)?;

        let mut args = parsers::any_to_metacall(args);

        Ok(parsers::metacall_to_any(unsafe {
            metacallv_class(
                *self.class.clone(),
                c_key.as_ptr(),
                args.as_mut_ptr(),
                args.len(),
            )
        }))
    }

    pub fn into_raw(self) -> *mut c_void {
        *self.class
    }
}
