use crate::{
    bindings::{metacall_destroy, metacall_initialize},
    types::MetaCallInitError,
};
use std::ffi::c_int;

pub fn destroy_manually() -> c_int {
    unsafe { metacall_destroy() }
}
pub fn initialize_manually() -> c_int {
    unsafe { metacall_initialize() }
}

pub struct MetaCallAutoDestroy(pub fn() -> c_int);
impl Drop for MetaCallAutoDestroy {
    fn drop(&mut self) {
        self.0();
    }
}

/// Initializes MetaCall. Always remember to store the output in a variable to avoid instant drop.
/// For example: ...
/// ```
/// // Initialize metacall at the top of your main function before loading your codes or
/// // calling any function.
/// let _metacall = metacall::initialize().unwrap();
///
///
/// ```
pub fn initialize() -> Result<MetaCallAutoDestroy, MetaCallInitError> {
    if initialize_manually() != 0 {
        return Err(MetaCallInitError::new());
    }

    Ok(MetaCallAutoDestroy(destroy_manually))
}
