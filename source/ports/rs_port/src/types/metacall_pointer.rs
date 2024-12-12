use super::MetaCallValue;
use crate::{
    bindings::{metacall_value_create_ptr, metacall_value_destroy, metacall_value_to_ptr},
    MetaCallNull,
};
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

/// Represents MetaCall pointer. This type cannot be used in other languages. This type is highly
/// unsafe so be careful!
pub struct MetaCallPointer {
    leak: bool,
    rust_value: *mut Box<dyn MetaCallValue>,
    rust_value_leak: bool,
    value: *mut c_void,
}
unsafe impl Send for MetaCallPointer {}
unsafe impl Sync for MetaCallPointer {}
impl Clone for MetaCallPointer {
    fn clone(&self) -> Self {
        Self {
            leak: true,
            rust_value: self.rust_value,
            rust_value_leak: true,
            value: self.value,
        }
    }
}
impl Debug for MetaCallPointer {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let boxed_value = unsafe { Box::from_raw(self.rust_value) };
        let value = if (*boxed_value).is::<MetaCallNull>() {
            None
        } else {
            Some(format!("{:#?}", boxed_value))
        };
        Box::leak(boxed_value);

        f.debug_struct("MetaCallPointer")
            .field("value", &value)
            .finish()
    }
}

impl MetaCallPointer {
    fn get_rust_value_ptr(value: *mut c_void) -> *mut Box<dyn MetaCallValue> {
        unsafe { metacall_value_to_ptr(value) as *mut Box<dyn MetaCallValue> }
    }

    #[doc(hidden)]
    pub fn new_raw(value: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self {
            leak: false,
            rust_value: Self::get_rust_value_ptr(value),
            rust_value_leak: false,
            value,
        })
    }

    #[doc(hidden)]
    pub fn new_raw_leak(value: *mut c_void) -> Result<Self, Box<dyn MetaCallValue>> {
        Ok(Self {
            leak: true,
            rust_value: Self::get_rust_value_ptr(value),
            rust_value_leak: false,
            value,
        })
    }

    /// Creates a new MetaCall pointer and casts it to T.
    pub fn new(ptr: impl MetaCallValue) -> Self {
        let rust_value = Box::into_raw(Box::new(Box::new(ptr) as Box<dyn MetaCallValue>));

        Self {
            leak: false,
            rust_value,
            rust_value_leak: false,
            value: unsafe { metacall_value_create_ptr(rust_value.cast()) },
        }
    }

    /// Consumes the MetaCall pointer and returns ownership of the value without type
    /// casting([MetaCallValue](MetaCallValue)).
    pub fn get_value_untyped(mut self) -> Box<dyn MetaCallValue> {
        self.rust_value_leak = true;

        unsafe { *Box::from_raw(self.rust_value) }
    }
    /// Consumes the MetaCall pointer and returns ownership of the value.
    pub fn get_value<T: MetaCallValue>(self) -> Result<T, Box<dyn MetaCallValue>> {
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

impl Drop for MetaCallPointer {
    fn drop(&mut self) {
        if !self.leak {
            unsafe { metacall_value_destroy(self.value) }
        }

        if !self.rust_value_leak {
            unsafe { drop(Box::from_raw(self.rust_value)) }
        }
    }
}
