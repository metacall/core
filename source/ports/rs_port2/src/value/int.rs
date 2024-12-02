use std::marker::PhantomData;

use anyhow::Result;
use metacall_bindings::value::{
    create::{metacall_value_create_int, metacall_value_create_long, metacall_value_create_short},
    metacall_value_to_int, metacall_value_to_long, metacall_value_to_short,
};

use crate::check_null_ptr;

use super::{Value, ValueError};

impl Value<i64> {
    pub fn new(value: i64) -> Result<Self> {
        let ptr = unsafe { metacall_value_create_long(value) };
        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }
    pub fn get_value(&self) -> Result<i64> {
        let res = unsafe { metacall_value_to_long(self.ptr) };

        Ok(res)
    }
}

impl Value<i32> {
    pub fn new(value: i32) -> Result<Self> {
        let ptr = unsafe { metacall_value_create_int(value.into()) };
        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }

    pub fn get_value(&self) -> Result<i32> {
        let res = unsafe { metacall_value_to_int(self.ptr) };

        Ok(res)
    }
}

impl Value<i16> {
    pub fn new(value: i16) -> Result<Self> {
        let ptr = unsafe { metacall_value_create_short(value.into()) };
        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }

    pub fn get_value(&self) -> Result<i16> {
        let res: i16 = unsafe { metacall_value_to_short(self.ptr) }.try_into()?;

        Ok(res)
    }
}
