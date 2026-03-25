use super::loader::{self, LoadingMethod};
use compiler::{memory::MemoryRegistration, RegistrationError};
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};

/// # Safety
///
/// This function is called from C code. Both `name` and `buffer` must be valid,
/// non-null pointers to null-terminated C strings containing valid UTF-8.
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    name: *const c_char,
    buffer: *const c_char,
    _size: usize,
) -> *mut c_void {
    let result = std::panic::catch_unwind(|| {
        if name.is_null() || buffer.is_null() {
            eprintln!("rs_loader_impl_load_from_memory: received null pointer for name or buffer");
            return std::ptr::null_mut();
        }

        let name = match CStr::from_ptr(name).to_str() {
            Ok(s) => s.to_owned(),
            Err(_) => {
                eprintln!("rs_loader_impl_load_from_memory: name is not valid UTF-8");
                return std::ptr::null_mut();
            }
        };
        let code = match CStr::from_ptr(buffer).to_str() {
            Ok(s) => s.to_owned(),
            Err(_) => {
                eprintln!("rs_loader_impl_load_from_memory: buffer is not valid UTF-8");
                return std::ptr::null_mut();
            }
        };
        let instance = LoadingMethod::Memory(match MemoryRegistration::new(name, code) {
            Ok(instance) => instance,
            Err(error) => match error {
                RegistrationError::CompilationError(analysis_error) => {
                    return loader::load_on_error(analysis_error);
                }
                RegistrationError::DynlinkError(dynlink_error) => {
                    return loader::load_on_error(dynlink_error);
                }
            },
        });
        Box::into_raw(Box::new(vec![instance])) as *mut c_void
    });
    result.unwrap_or(std::ptr::null_mut())
}
