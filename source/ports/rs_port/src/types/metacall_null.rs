use crate::bindings::metacall_value_create_null;
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

#[derive(Clone)]
/// Represents NULL.
// This is a zero-sized struct. It doesn't allocate any memory and will only create a null pointer
// when needed.
pub struct MetacallNull();
unsafe impl Send for MetacallNull {}
unsafe impl Sync for MetacallNull {}
impl Debug for MetacallNull {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetacallNull {{ }}")
    }
}

impl MetacallNull {
    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_null() }
    }
}
