use std::{
    ffi::{c_char, CStr},
    marker::PhantomData,
};

use anyhow::Result;
use metacall_bindings::value::{create::metacall_value_create_string, metacall_value_to_string};

use crate::{check_null_ptr, impl_value_constructor};

use super::{Value, ValueError};

impl Value<&str> {
    pub fn get_value<'a>(&self) -> Result<&'a str> {
        let ptr = unsafe { metacall_value_to_string(self.ptr) };
        check_null_ptr!(ptr, ValueError::NullPointer);
        unsafe {
            dbg!(&CStr::from_ptr(ptr));
        }

        let str = unsafe { CStr::from_ptr(ptr) }.to_str();
        Ok(str.unwrap())
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
        let value = value.to_string();
        let ptr = value.as_ptr() as *const c_char;
        let ptr = unsafe {
            metacall_value_create_string(ptr, value.len())
        };

        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    },
    String,
    &str
);
