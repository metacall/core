use std::{ffi::CStr, marker::PhantomData};

use anyhow::Result;
use metacall_bindings::value::{create::metacall_value_create_string, metacall_value_to_string};

use crate::{check_null_ptr, impl_value_constructor};

use super::{Value, ValueError};

impl Value<&str> {
    pub fn get_value(&self) -> Result<&str> {
        let ptr = unsafe { metacall_value_to_string(self.ptr) };

        check_null_ptr!(ptr, ValueError::NullPointer);

        let str = unsafe { CStr::from_ptr(ptr) }.to_str()?;
        Ok(str)
    }
}

impl Value<String> {
    pub fn get_value(&self) -> Result<String> {
        let ptr = unsafe { metacall_value_to_string(self.ptr) };

        check_null_ptr!(ptr, ValueError::NullPointer);

        let str = unsafe { CStr::from_ptr(ptr) }.to_str()?;
        Ok(str.to_string())
    }
}

impl_value_constructor!(
    value => {
        let ptr =
            unsafe { metacall_value_create_string(value.as_ptr() as *const i8, value.len()  ) };
        check_null_ptr!(ptr, ValueError::NullPointer);
        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    },
    String,
    &str
);
