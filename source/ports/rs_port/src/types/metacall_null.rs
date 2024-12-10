use crate::bindings::metacall_value_create_null;
use std::{
    ffi::c_void,
    fmt::{self, Debug, Formatter},
};

#[derive(Clone)]
/// Represents NULL.
// This is a zero-sized struct. It doesn't allocate any memory and will only create a null pointer
// when needed.
pub struct MetaCallNull();
unsafe impl Send for MetaCallNull {}
unsafe impl Sync for MetaCallNull {}
impl Debug for MetaCallNull {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCallNull {{ }}")
    }
}

impl MetaCallNull {
    #[doc(hidden)]
    pub fn into_raw(self) -> *mut c_void {
        unsafe { metacall_value_create_null() }
    }
}
