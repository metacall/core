use super::loader::{self, LoadingMethod};
use compiler::{memory::MemoryRegistration, RegistrationError};
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    name: *const c_char,
    buffer: *const c_char,
    _size: usize,
) -> *mut c_void {
    let name = unsafe { CStr::from_ptr(name) }
        .to_str()
        .expect("Unable to cast CStr to str")
        .to_owned();
    let code = unsafe { CStr::from_ptr(buffer) }
        .to_str()
        .expect("Unable to cast CStr to str")
        .to_owned();
    let instance = LoadingMethod::Memory(match MemoryRegistration::new(name, code) {
        Ok(instance) => instance,
        Err(error) => match error {
            RegistrationError::CompilationError(analysis_error) => {
                return loader::load_on_error(analysis_error);
            }
            RegistrationError::DlopenError(dlopen_error) => {
                return loader::load_on_error(dlopen_error);
            }
        },
    });
    Box::into_raw(Box::new(vec![instance])) as *mut c_void
}
