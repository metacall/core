use std::{any::type_name, ffi::{CStr, CString}, fmt, marker::PhantomData, os::raw::c_void, ptr::null_mut};

use crate::bindings::{metacall_value_copy, metacall_value_create_bool, metacall_value_create_char, metacall_value_create_double, metacall_value_create_float, metacall_value_create_int, metacall_value_create_long, metacall_value_create_short, metacall_value_create_string, metacall_value_destroy, metacall_value_id, metacall_value_type_name};

#[derive(Debug)]
pub struct Value<T> {
    ptr: *mut c_void,
    _phantom: PhantomData<T>,
}

impl<T> Clone for Value<T> {
    fn clone(&self) -> Self {
        Self {
            ptr: unsafe { metacall_value_copy(self.ptr) },
            _phantom: self._phantom
        }
    }
}

impl<T> Drop for Value<T> {
    fn drop(&mut self) {
        unsafe {
            metacall_value_destroy(self.ptr);
        }
        self.ptr = null_mut();
    }
}

impl<T> fmt::Display for Value<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // Use `type_name::<T>()` to get the type name as a string
        write!(f, "Value<{}> of type {}: {}", type_name::<T>(), self.id_name(), "self.get().to_string()") // TODO
    }
}

impl<T> Value<T> {
    /// A simple method to check the nullity of the Value.
    pub fn is_null(&self) -> bool {
        self.ptr.is_null()
    }

    /// Provides the type identifier (ID).
    pub fn id(&self) -> metacall_value_id {
        unsafe { metacall_value_id(self.ptr) }
    }

    /// Provides the type identifier (ID) in a readable form (as a `String`).
    pub fn id_name(&self) -> String {
        let ptr = unsafe { metacall_value_type_name(self.ptr) };
        let c_str = unsafe { CStr::from_ptr(ptr) }.to_str();
        c_str.unwrap().to_string()
    }
}

impl Value<bool> {
    pub fn new(b: bool) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_bool(b.into()) },
            _phantom: PhantomData,
        }
    }
}

impl Value<i8> {
    pub fn new(c: i8) -> Self {
        Self {
            ptr: unsafe {  metacall_value_create_char(c) },
            _phantom: PhantomData,
        }
    }
}

impl Value<i16> {
    pub fn new(s: i16) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_short(s) },
            _phantom: PhantomData,
        }
    }
}

impl Value<i32> {
    pub fn new(i: i32) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_int(i) },
            _phantom: PhantomData,
        }
    }
}

impl Value<i64> {
    pub fn new(l: i64) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_long(l) },
            _phantom: PhantomData,
        }
    }
}

impl Value<f32> {
    pub fn new(f: f32) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_float(f) },
            _phantom: PhantomData,
        }
    }
}

impl Value<f64> {
    pub fn new(d: f64) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_double(d) },
            _phantom: PhantomData,
        }
    }
}

impl Value<&str> {
    pub fn new(s: &str) -> Self {
        let c_string = CString::new(s).unwrap();
        let c_str = c_string.as_c_str();
        Self {
            ptr: unsafe { metacall_value_create_string(c_str.as_ptr(), s.len()) },
            _phantom: PhantomData,
        }
    }
}

impl Value<String> {
    pub fn new(s: String) -> Self {
        let length = s.len();
        let c_string = CString::new(s).unwrap();
        let c_str = c_string.as_c_str();
        Self {
            ptr: unsafe { metacall_value_create_string(c_str.as_ptr(), length) },
            _phantom: PhantomData,
        }
    }
}
