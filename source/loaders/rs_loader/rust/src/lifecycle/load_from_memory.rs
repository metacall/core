use super::loader::{self, LoadingMethod};
use compiler::{memory::MemoryRegistration, RegistrationError};
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};
use std::panic::catch_unwind;

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    name: *const c_char,
    buffer: *const c_char,
    _size: usize,
) -> *mut c_void {
    let result = catch_unwind(|| {
        if name.is_null() || buffer.is_null() {
            return std::ptr::null_mut();
        }

        let name_str = match CStr::from_ptr(name).to_str() {
            Ok(s) => s.to_owned(),
            Err(_) => return std::ptr::null_mut(),
        };

        let code_str = match CStr::from_ptr(buffer).to_str() {
            Ok(s) => s.to_owned(),
            Err(_) => return std::ptr::null_mut(),
        };

        let instance = match MemoryRegistration::new(name_str, code_str) {
            Ok(inst) => LoadingMethod::Memory(inst),
            Err(error) => {
                return match error {
                    RegistrationError::CompilationError(e) => loader::load_on_error(e),
                    RegistrationError::DynlinkError(e) => loader::load_on_error(e),
                };
            }
        };

        Box::into_raw(Box::new(vec![instance])) as *mut c_void
    });

    result.unwrap_or(std::ptr::null_mut())
}