use std::marker::PhantomData;

use crate::{check_null_ptr, impl_value_constructor};

use super::{Value, ValueError};
use anyhow::Result;
use metacall_bindings::value::{
    create::{metacall_value_create_double, metacall_value_create_float},
    metacall_value_to_double, metacall_value_to_float,
};
impl Value<f32> {
    pub fn get_value(&self) -> Result<f32> {
        let res = unsafe { metacall_value_to_float(self.ptr) };
        Ok(res)
    }
}

impl_value_constructor!(
    value => {
        let ptr = unsafe { metacall_value_create_float(value) };
        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }, f32
);

impl Value<f64> {

    pub fn get_value(&self) -> Result<f64> {
        let res = unsafe { metacall_value_to_double(self.ptr) };
        Ok(res)
    }
}

impl_value_constructor!(
    value => {
        let ptr = unsafe { metacall_value_create_double(value) };
        check_null_ptr!(ptr, ValueError::NullPointer);

        Ok(Self {
            ptr,
            _phantom: PhantomData,
        })
    }, f64
);
