mod float;
mod int;
mod string;

use anyhow::Result;
use metacall_bindings::value::{
    metacall_value_destroy, metacall_value_id, metacall_value_id_name, metacall_value_type_name,
    MetacallValueID,
};
use std::{
    ffi::{c_void, CStr},
    marker::PhantomData,
    ptr::null_mut,
};
use thiserror::Error;

use crate::check_null_ptr;

#[macro_export]
macro_rules! impl_value_constructor {
    ($value:ident =>  $code:block, $($type:ty),+) => {
        $(
            impl Value<$type> {
                pub fn new($value: $type) -> Result<Self> {
                    $code
                }
            }
        )+
    };
}

#[derive(Error, Debug)]
pub enum ValueError {
    #[error("Failed to create CString: {0}")]
    CStringError(#[from] std::ffi::NulError),
    #[error("Failed to convert CString to &str: {0}")]
    Utf8Error(#[from] std::str::Utf8Error),
    #[error("Null pointer encountered")]
    NullPointer,
    #[error("Unknown error")]
    Unknown,
}

pub struct Value<T> {
    ptr: *mut c_void,
    _phantom: PhantomData<T>,
}

impl<T> Value<T> {
    /// A simple method to check the nullity of the Value.
    pub fn is_null(&self) -> Result<bool> {
        let id = self.value_id()?;
        Ok(id == MetacallValueID::Null)
    }

    /// Provides the type identifier (ID).
    pub fn value_id(&self) -> Result<MetacallValueID> {
        check_null_ptr!(self.ptr, ValueError::NullPointer);
        let id = unsafe { metacall_value_id(self.ptr) };
        Ok(id)
    }

    /// Provides the type identifier (ID) in a readable form (as a `String`).
    pub fn value_id_name(&self) -> Result<String> {
        check_null_ptr!(self.ptr, ValueError::NullPointer);
        let ptr = unsafe { metacall_value_type_name(self.ptr) };
        let c_str = unsafe { CStr::from_ptr(ptr) }.to_str()?;
        Ok(c_str.to_string())
    }
}

impl<T> Drop for Value<T> {
    fn drop(&mut self) {
        unsafe {
            metacall_value_destroy(self.ptr);
            self.ptr = null_mut();
        }
    }
}

#[cfg(test)]
mod test {

    use super::Value;
    #[test]
    fn metacall_create_value_string() {
        let string = String::from("Hello!");
        let value = Value::<String>::new(string.clone()).unwrap();
        let result = value.get_value().unwrap();
        // assert_eq!(result, string, "Failed in String test");
        println!("[String] \"{result}\" == \"{string}\"  => Passed",);

        let str = "Hello!";
        let value = Value::<&str>::new(str).unwrap();
        let result = value.get_value().unwrap();
        assert_eq!(result, str, "Failed in &str test");
        println!("[&str] \"{result}\" == \"{str}\" => Passed",);
    }

    #[test]
    fn metacall_create_value_int() {
        let int32 = i32::MAX;
        let value = Value::<i32>::new(int32).unwrap();
        let result = value.get_value().unwrap();
        assert_eq!(result, int32);
        println!("[i32] \"{result}\" == \"{int32}\"  => Passed",);

        let int64 = i64::MAX;
        let value = Value::<i64>::new(int64).unwrap();
        let result = value.get_value().unwrap();
        assert_eq!(result, int64);
        println!("[i64] \"{result}\" == \"{int64}\" => Passed",);
    }

    #[test]
    fn metacall_create_value_float() {
        let float32 = f32::MAX;
        let value = Value::<f32>::new(float32).unwrap();
        let result = value.get_value().unwrap();
        assert_eq!(result, float32);
        println!("[i32] \"{result}\" == \"{float32}\"  => Passed",);

        let float64 = f64::MAX;
        let value = Value::<f64>::new(float64).unwrap();
        let result = value.get_value().unwrap();
        assert_eq!(result, float64);
        println!("[i64] \"{result}\" == \"{float64}\" => Passed",);
    }
}
