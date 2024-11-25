use std::marker::PhantomData;

use anyhow::Result;
use metacall_bindings::value::{
    create::{metacall_value_create_int, metacall_value_create_long},
    metacall_value_to_int, metacall_value_to_long,
};

use crate::{check_null_ptr, impl_value_constructor};

use super::{Value, ValueError};

impl Value<i64> {
    pub fn get_value(&self) -> Result<i64> {
        let res = unsafe { metacall_value_to_long(self.ptr).to_owned() };

        Ok(res)
    }
}

impl_value_constructor!(
    value => {
        let ptr = unsafe { metacall_value_create_long(value) };

        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }, i64
);

impl Value<i32> {
    pub fn get_value(&self) -> Result<i32> {
        let res = unsafe { metacall_value_to_int(self.ptr).to_owned() };

        Ok(res)
    }
}

impl Value<i16> {
    pub fn get_value(&self) -> Result<i16> {
        let res: i16 = unsafe { metacall_value_to_int(self.ptr).to_owned() }.try_into()?;

        Ok(res)
    }
}

impl_value_constructor!(
    value => {
        let ptr = unsafe { metacall_value_create_int(value.into()) };

        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }, i16, i32
);
