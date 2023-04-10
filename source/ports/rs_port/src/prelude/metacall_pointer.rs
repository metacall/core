use super::MetacallObjectProtocol;
use crate::bindings::{metacall_value_create_ptr, metacall_value_destroy, metacall_value_to_ptr};
use std::{ffi::c_void, sync::Arc};

#[derive(Clone, Debug)]
pub struct MetacallPointer {
    ptr: Arc<*mut c_void>,
}
unsafe impl Send for MetacallPointer {}
unsafe impl Sync for MetacallPointer {}

impl MetacallPointer {
    pub fn new_raw(ptr: *mut c_void) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        Ok(Self { ptr: Arc::new(ptr) })
    }

    pub fn new(
        ptr: Box<dyn MetacallObjectProtocol>,
    ) -> Result<Self, Box<dyn MetacallObjectProtocol>> {
        let ptr = Box::into_raw(ptr) as *mut _ as *mut c_void;
        Ok(Self {
            ptr: Arc::new(unsafe { metacall_value_create_ptr(ptr) }),
        })
    }

    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_to_ptr(*self.ptr) }
    }
}

impl Drop for MetacallPointer {
    fn drop(&mut self) {
        unsafe { metacall_value_destroy(*self.ptr) }
    }
}
