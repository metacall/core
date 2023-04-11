use crate::bindings::metacall_value_create_null;
use std::ffi::c_void;

#[derive(Clone, Debug)]
pub struct MetacallNull();
unsafe impl Send for MetacallNull {}
unsafe impl Sync for MetacallNull {}

impl MetacallNull {
    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_null() }
    }
}
