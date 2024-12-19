use crate::{
    bindings::{metacall_destroy, metacall_initialize, metacall_is_initialized},
    types::MetaCallInitError,
};
use std::{ffi::c_int, ptr};

pub struct MetaCallDestroy(unsafe extern "C" fn() -> c_int);

impl Drop for MetaCallDestroy {
    fn drop(&mut self) {
        let code = unsafe { self.0() };

        if code != 0 {
            panic!("MetaCall failed to destroy with code: {}", code)
        }
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
pub fn initialize() -> Result<MetaCallDestroy, MetaCallInitError> {
    let code = unsafe { metacall_initialize() };

    if code != 0 {
        return Err(MetaCallInitError::new(code));
    }

    Ok(MetaCallDestroy(metacall_destroy))
}

pub fn is_initialized() -> bool {
    let initialized = unsafe { metacall_is_initialized(ptr::null_mut()) };

    if initialized == 0 {
        return true;
    }

    false
}
