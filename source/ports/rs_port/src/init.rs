use crate::bindings::{metacall_initialize, metacall_destroy, metacall_is_initialized};
use std::{os::raw::c_int, fmt, ptr, sync::Once};

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

#[cfg(test)]
static INITIALIZED: std::sync::atomic::AtomicBool = std::sync::atomic::AtomicBool::new(false);

impl Drop for MetaCallDestroy {
    fn drop(&mut self) {
        let result = unsafe { self.0() };

        if result != 0 {
            panic!("MetaCall failed to destroy with code: {}", result)
        }

        #[cfg(test)]
        {
            INITIALIZED.store(false, std::sync::atomic::Ordering::SeqCst);
        }
    }
}

/// Initializes Metacall. Always remember to store the output in a variable to avoid instant drop.
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

pub fn init() {
    static INIT: Once = Once::new();
    static mut FINI: Option<MetaCallDestroy> = None;

    INIT.call_once(|| {

        unsafe {
            FINI = Some(initialize().unwrap())
        };

        #[cfg(test)]
        {
            INITIALIZED.store(true, std::sync::atomic::Ordering::SeqCst);
        }
    });
}

/// An exported constructor function. On supported platforms, this will be
/// invoked automatically before the program's `main` is called. This is done
/// for the convenience of library users since otherwise the thread-safety rules
/// around initialization can be difficult to fulfill.
///
/// This is a hidden public item to ensure the symbol isn't optimized away by a
/// rustc/LLVM bug: https://github.com/rust-lang/rust/issues/47384. As long as
/// any item in this module is used by the final binary (which `init` will be)
/// then this symbol should be preserved.
#[used]
#[doc(hidden)]
#[cfg_attr(
    any(target_os = "linux", target_os = "freebsd", target_os = "android"),
    link_section = ".init_array"
)]
#[cfg_attr(target_os = "macos", link_section = "__DATA,__mod_init_func")]
#[cfg_attr(target_os = "windows", link_section = ".CRT$XCU")]
pub static INIT_CTOR: extern "C" fn() = {
    #[cfg_attr(
        any(target_os = "linux", target_os = "android"),
        link_section = ".text.startup"
    )]
    extern "C" fn init_ctor() {
        init();
    }

    init_ctor
};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[cfg(any(
        target_os = "linux",
        target_os = "macos",
        target_os = "windows",
        target_os = "freebsd",
        target_os = "android"
    ))]
    fn is_initialized_before_main() {
        assert!(INITIALIZED.load(std::sync::atomic::Ordering::SeqCst));
    }
}
