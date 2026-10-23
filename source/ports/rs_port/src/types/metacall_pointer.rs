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
        let rust_value = if self.rust_value.is_null() {
            std::ptr::null_mut()
        } else {
            let cloned_value = unsafe { (&*self.rust_value).clone() };
            Box::into_raw(Box::new(cloned_value))
        };

        Self {
            leak: true,
            rust_value,
            rust_value_leak: false,
            value: self.value,
        }
    }
}
impl Debug for MetaCallPointer {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let value = if self.rust_value.is_null() {
            None
        } else {
            let value_ref = unsafe { &*self.rust_value };
            if value_ref.is::<MetaCallNull>() {
                None
            } else {
                Some(format!("{:#?}", value_ref))
            }
        };

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
        if self.rust_value.is_null() {
            self.rust_value_leak = true;
            return Box::new(MetaCallNull());
        }

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
        if !self.leak && !self.value.is_null() {
            unsafe { metacall_value_destroy(self.value) }
        }

        if !self.rust_value_leak && !self.rust_value.is_null() {
            unsafe { drop(Box::from_raw(self.rust_value)) }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::MetaCallPointer;

    #[test]
    fn clone_and_consume_value_does_not_share_owned_box() {
        let original = MetaCallPointer::new(String::from("hi"));
        let cloned = original.clone();

        let value1 = cloned.get_value::<String>().unwrap();
        let value2 = original.get_value::<String>().unwrap();

        assert_eq!(value1, "hi");
        assert_eq!(value2, "hi");
    }
}
