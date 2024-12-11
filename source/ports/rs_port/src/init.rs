use crate::bindings::{metacall_initialize, metacall_destroy, metacall_is_initialized};
use std::{os::raw::c_int, fmt, ptr};

#[derive(Debug, Clone)]
/// This error happens when it's not possible to initialize the MetaCall Core. You can check
/// your logs for more information.
pub struct InitializationError(c_int);
impl InitializationError {
    #[doc(hidden)]
    pub fn new(result: c_int) -> Self {
        Self(result)
    }
}
impl fmt::Display for InitializationError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "MetaCall initialization failed with code {}", self.0)
    }
}

pub struct MetaCallDestroy(unsafe extern "C" fn() -> c_int);

impl Drop for MetaCallDestroy {
    fn drop(&mut self) {
        let result = unsafe { self.0() };

        if result != 0 {
            panic!("MetaCall failed to destroy with code: {}", result)
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
pub fn initialize() -> Result<MetaCallDestroy, InitializationError> {
    let result = unsafe { metacall_initialize() };

    if result != 0 {
        return Err(InitializationError::new(result));
    }

    Ok(MetaCallDestroy(metacall_destroy))
}

pub fn is_initialized() -> bool {
    let initialized = unsafe { metacall_is_initialized(ptr::null_mut()) };

    if initialized == 0 {
        return true
    }

    false
}
