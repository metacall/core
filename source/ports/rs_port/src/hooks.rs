use crate::{
    bindings::{metacall_destroy, metacall_initialize},
    prelude::MetacallInitError,
};
use std::ffi::c_int;

pub fn destroy_manually() -> c_int {
    unsafe { metacall_destroy() }
}
pub fn initialize_manually() -> c_int {
    unsafe { metacall_initialize() }
}

pub struct MetacallAutoDestroy(pub fn() -> c_int);
impl Drop for MetacallAutoDestroy {
    fn drop(&mut self) {
        self.0();
    }
}

pub fn initialize() -> Result<MetacallAutoDestroy, MetacallInitError> {
    if initialize_manually() != 0 {
        return Err(MetacallInitError::new());
    }

    Ok(MetacallAutoDestroy(destroy_manually))
}
