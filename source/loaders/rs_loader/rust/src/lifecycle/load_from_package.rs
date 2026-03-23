use super::loader::{self, LoadingMethod};
use compiler::{package::PackageRegistration, RegistrationError};
use std::os::raw::{c_char, c_void};
use std::path::PathBuf;

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_package(
    loader_impl: *mut c_void,
    path: *mut *const c_char,
) -> *mut c_void {
    loader::load(
        loader_impl,
        path,
        1,
        false,
        |path_buf: PathBuf, load_on_error: loader::LoadOnErrorPointer| -> Result<LoadingMethod, *mut c_void> {
            let instance = PackageRegistration::new(path_buf).map_err(|error| match error {
                RegistrationError::CompilationError(e) => load_on_error(e),
                RegistrationError::DynlinkError(e) => load_on_error(e),
            })?;

            Ok(LoadingMethod::Package(instance))
        },
    )
}
