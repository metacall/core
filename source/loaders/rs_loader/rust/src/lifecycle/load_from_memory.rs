use super::loader::{self, LoadingMethod};
use super::template::instantiations_from_data;
use compiler::{memory::MemoryRegistration, RegistrationError};
use std::ffi::CStr;
use std::os::raw::{c_char, c_void};

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    name: *const c_char,
    buffer: *const c_char,
    _size: usize,
    data: *mut c_void,
) -> *mut c_void {
    if name.is_null() {
        return loader::load_on_error("Rust loader received a null name pointer.");
    }

    if buffer.is_null() {
        return loader::load_on_error("Rust loader received a null buffer pointer.");
    }

    let name = match CStr::from_ptr(name).to_str() {
        Ok(name) => name.to_owned(),
        Err(_) => return loader::load_on_error("Rust loader received an invalid UTF-8 name."),
    };

    let code = match CStr::from_ptr(buffer).to_str() {
        Ok(code) => code.to_owned(),
        Err(_) => return loader::load_on_error("Rust loader received invalid UTF-8 source code."),
    };
    let instantiations = match instantiations_from_data(data) {
        Ok(instantiations) => instantiations,
        Err(error) => return loader::load_on_error(error),
    };
    let instance =
        LoadingMethod::Memory(match MemoryRegistration::new(name, code, &instantiations) {
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
}
