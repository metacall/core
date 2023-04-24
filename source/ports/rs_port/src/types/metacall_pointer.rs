use super::MetacallValue;
use crate::{
    bindings::{metacall_value_create_ptr, metacall_value_destroy, metacall_value_to_ptr},
    MetacallNull,
};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

/// Represents Metacall pointer. This type cannot be used in other languages. This type is highly
/// unsafe so be careful!
pub struct MetacallPointer {
    leak: bool,
    rust_value: *mut Box<dyn MetacallValue>,
    rust_value_leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetacallPointer {}
unsafe impl Sync for MetacallPointer {}
impl Clone for MetacallPointer {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            rust_value: self.rust_value.clone(),
            rust_value_leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetacallPointer {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let boxed_value = unsafe { Box::from_raw(self.rust_value) };
        let value = if (*boxed_value).is::<MetacallNull>() {
            None
        } else {
            Some(format!("{:#?}", boxed_value))
        };
        Box::leak(boxed_value);

        f.debug_struct("MetacallPointer")
            .field("value", &value)
            .finish()
    }
}

impl MetacallPointer {
    fn get_rust_value_ptr(value: *mut c_void) -> *mut Box<dyn MetacallValue> {
        unsafe { metacall_value_to_ptr(value) as *mut Box<dyn MetacallValue> }
    }

    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self {
            leak: false,
            rust_value: Self::get_rust_value_ptr(value),
            rust_value_leak: false,
            value,
        })
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Result<Self, Box<dyn MetacallValue>> {
        Ok(Self {
            leak: true,
            rust_value: Self::get_rust_value_ptr(value),
            rust_value_leak: false,
            value,
        })
    }

    /// Creates a new Metacall pointer and casts it to T.
    pub fn new(ptr: impl MetacallValue) -> Self {
        let rust_value = Box::into_raw(Box::new(Box::new(ptr) as Box<dyn MetacallValue>));

        Self {
            leak: false,
            rust_value,
            rust_value_leak: false,
            value: unsafe { metacall_value_create_ptr(rust_value.cast()) },
        }
    }

    /// Consumes the Metacall pointer and returns ownership of the value without type
    /// casting([MetacallValue](MetacallValue)).
    pub fn get_value_untyped(mut self) -> Box<dyn MetacallValue> {
        self.rust_value_leak = true;

        unsafe { *Box::from_raw(self.rust_value) }
    }
    /// Consumes the Metacall pointer and returns ownership of the value.
    pub fn get_value<T: MetacallValue>(self) -> Result<T, Box<dyn MetacallValue>> {
        match self.get_value_untyped().downcast::<T>() {
            Ok(rust_value) => Ok(rust_value),
            Err(original) => Err(original),
        }
    }

    #[doc(hidden)]
    pub fn into_raw(mut self) -> *mut c_void {
        self.leak = true;
        self.rust_value_leak = true;

        self.value
    }
}

impl Drop for MetacallPointer {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }

        if !self.rust_value_leak {
            unsafe { drop(Box::from_raw(self.rust_value)) }
        }
    }
}
