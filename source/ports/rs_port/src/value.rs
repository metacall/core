use std::{any::{type_name, Any}, collections::HashMap, ffi::{CStr, CString}, fmt, marker::PhantomData, os::raw::c_void, ptr::null_mut};

use crate::bindings::{metacall_value_copy, metacall_value_count, metacall_value_create_array, metacall_value_create_bool, metacall_value_create_buffer, metacall_value_create_char, metacall_value_create_double, metacall_value_create_float, metacall_value_create_int, metacall_value_create_long, metacall_value_create_map, metacall_value_create_ptr, metacall_value_create_short, metacall_value_create_string, metacall_value_destroy, metacall_value_from_array, metacall_value_from_bool, metacall_value_from_buffer, metacall_value_from_char, metacall_value_from_double, metacall_value_from_float, metacall_value_from_int, metacall_value_from_long, metacall_value_from_ptr, metacall_value_from_short, metacall_value_from_string, metacall_value_id, metacall_value_size, metacall_value_type_name};

pub struct Value<T> where T: Any + Clone {
    ptr: *mut c_void,
    _phantom: PhantomData<T>,
}

impl<T> Clone for Value<T> where T: Any + Clone {
    fn clone(&self) -> Self {
        unsafe {
            Self {
                ptr: metacall_value_copy(self.ptr),
                _phantom: PhantomData,
            }
        }
    }
}

impl<T> Drop for Value<T> where T: Any + Clone {
    fn drop(&mut self) {
        unsafe {
            metacall_value_destroy(self.ptr);
        }
        self.ptr = null_mut();
    }
}

impl<T> fmt::Display for Value<T> where T: Any + Clone {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        // Use `type_name::<T>()` to get the type name as a string
        write!(f, "Value<{}> of type {}: {}", type_name::<T>(), self.id_name(), "self.get().to_string()") // TODO
    }
}

impl<T> Value<T> where T: Any + Clone {
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

    /// Get the size of the value
    pub fn size(&self) -> usize {
        unsafe { metacall_value_size(self.ptr) }
    }

    /// Get the amount of values this value contains
    pub fn len(&self) -> usize {
        unsafe { metacall_value_count(self.ptr) }
    }

    /// Check if the value contains zero values
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }


    // /// Get the value.
    // pub fn downcast(&self) -> T {
    //     me
    // }

    /*
    // Check if the value is of type i32
    fn is_i32(&self) -> bool {
        self.value.type_id() == std::any::TypeId::of::<i32>()
    }

    // Attempt to downcast the value to i32
    fn downcast_to_i32(&self) -> Option<&i32> {
        self.value.downcast_ref::<i32>()
    }
    */
}

impl Value<bool> {
    pub fn new(b: bool) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_bool(b.into()) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, b: bool) {
        unsafe { metacall_value_from_bool(self.ptr, b.into()) };
    }
}

impl Value<i8> {
    pub fn new(c: i8) -> Self {
        Self {
            ptr: unsafe {  metacall_value_create_char(c) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, c: i8) {
        unsafe { metacall_value_from_char(self.ptr, c) };
    }
}

impl Value<i16> {
    pub fn new(s: i16) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_short(s) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, s: i16) {
        unsafe { metacall_value_from_short(self.ptr, s) };
    }
}

impl Value<i32> {
    pub fn new(i: i32) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_int(i) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, i: i32) {
        unsafe { metacall_value_from_int(self.ptr, i) };
    }
}

impl Value<i64> {
    pub fn new(l: i64) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_long(l) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, l: i64) {
        unsafe { metacall_value_from_long(self.ptr, l) };
    }
}

impl Value<f32> {
    pub fn new(f: f32) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_float(f) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, f: f32) {
        unsafe { metacall_value_from_float(self.ptr, f) };
    }
}

impl Value<f64> {
    pub fn new(d: f64) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_double(d) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, d: f64) {
        unsafe { metacall_value_from_double(self.ptr, d) };
    }
}

impl Value<String> {
    pub fn new(str: String) -> Self {
        let length = str.len();
        let c_string = CString::new(str).unwrap();
        let c_str = c_string.as_c_str();
        Self {
            ptr: unsafe { metacall_value_create_string(c_str.as_ptr(), length) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, str: String) {
        let length = str.len();
        let c_string = CString::new(str).unwrap();
        let c_str = c_string.as_c_str();
        unsafe { metacall_value_from_string(self.ptr, c_str.as_ptr(), length) };
    }
}

impl Value<Vec<i8>> {
    pub fn new(buffer: Vec<i8>) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_buffer(buffer.as_ptr() as *const c_void, buffer.len()) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, buffer: Vec<i8>) {
        unsafe { metacall_value_from_buffer(self.ptr, buffer.as_ptr() as *const c_void, buffer.len()) };
    }
}

impl<T> Value<Vec<Value<T>>> where T: Any + Clone {
    pub fn new(array: Vec<Value<T>>) -> Self {
        let mut array_ptr: Vec<*const c_void> = Vec::with_capacity(array.len());
        for value in array {
            array_ptr.push(value.ptr);
        }
        Self {
            ptr: unsafe { metacall_value_create_array(array_ptr.as_mut_ptr(), array_ptr.len()) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, array: Vec<Value<T>>) {
        let mut array_ptr: Vec<*const c_void> = Vec::with_capacity(array.len());
        for value in array {
            array_ptr.push(value.ptr);
        }
        unsafe { metacall_value_from_array(self.ptr, array_ptr.as_mut_ptr(), array_ptr.len()) };
    }
}

impl<T> Value<HashMap<String, Value<T>>> where T: Any + Clone {
    pub fn new(map: HashMap<String, Value<T>>) -> Self {
        let mut map_ptr: Vec<*const c_void> = Vec::with_capacity(map.len());
        for (key, value) in map {
            let key_value = Value::<String>::new(key);
            let mut tuple = vec![key_value.ptr as *const c_void, value.ptr as *const c_void];
            let tuple_ptr = unsafe { metacall_value_create_array(tuple.as_mut_ptr(), 2) };
            map_ptr.push(tuple_ptr);
        }
        Self {
            ptr: unsafe { metacall_value_create_map(map_ptr.as_mut_ptr(), map_ptr.len()) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, map: HashMap<String, Value<T>>) {
        let mut map_ptr: Vec<*const c_void> = Vec::with_capacity(map.len());
        for (key, value) in map {
            let key_value = Value::<String>::new(key);
            let mut tuple = vec![key_value.ptr as *const c_void, value.ptr as *const c_void];
            let tuple_ptr = unsafe { metacall_value_create_array(tuple.as_mut_ptr(), 2) };
            map_ptr.push(tuple_ptr);
        }
        unsafe { metacall_value_create_map(map_ptr.as_mut_ptr(), map_ptr.len()) };
    }
}

impl Value<*const dyn Any> {
    pub fn new(ptr: *const dyn Any) -> Self {
        Self {
            ptr: unsafe { metacall_value_create_ptr(ptr as *const c_void) },
            _phantom: PhantomData,
        }
    }

    pub fn from(&self, ptr: *const dyn Any) {
        unsafe { metacall_value_from_ptr(self.ptr, ptr as *const c_void) };
    }
}

// TODO
/*
metacall_value_create_future
metacall_value_create_function
metacall_value_create_function_closure
metacall_value_create_null
metacall_value_create_class
metacall_value_create_object
metacall_value_create_exception
metacall_value_create_throwable
*/
