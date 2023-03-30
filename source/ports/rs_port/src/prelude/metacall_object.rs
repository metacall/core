use super::{MetacallSetAttributeError, MetacallStringConversionError};
use crate::{
    bindings::{metacall_object_set, metacallv_object},
    cstring, cstring_enum,
    generated_bindings::metacall_object_get,
    parsers,
    prelude::Any,
};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Hash)]
pub struct MetacallObject {
    obj: Arc<*mut c_void>,
}
unsafe impl Send for MetacallObject {}
unsafe impl Sync for MetacallObject {}

impl MetacallObject {
    pub fn new(obj: *mut c_void) -> Self {
        Self { obj: Arc::new(obj) }
    }

    pub fn get_attribute<T: ToString>(
        &self,
        name: T,
    ) -> Result<Any, MetacallStringConversionError> {
        let c_name = cstring!(name)?;

        Ok(parsers::metacall_to_any(unsafe {
            metacall_object_get(*self.obj.clone(), c_name.as_ptr())
        }))
    }

    pub fn set_attribute<T: ToString>(
        &self,
        key: T,
        value: Any,
    ) -> Result<(), MetacallSetAttributeError> {
        let c_key = cstring_enum!(key, MetacallSetAttributeError)?;

        let c_args = parsers::any_to_metacall([value])[0];
        if unsafe { metacall_object_set(*self.obj.clone(), c_key.as_ptr(), c_args) } != 0 {
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
            metacallv_object(
                *self.obj.clone(),
                c_key.as_ptr(),
                args.as_mut_ptr(),
                args.len(),
            )
        }))
    }

    pub fn into_raw(self) -> *mut c_void {
        *self.obj
    }
}
