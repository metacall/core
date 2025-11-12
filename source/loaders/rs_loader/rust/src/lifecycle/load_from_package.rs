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
        |path_buf: PathBuf,
         load_on_error: loader::LoadOnErrorPointer|
         -> Result<LoadingMethod, *mut c_void> {
            Ok(LoadingMethod::Package(
                match PackageRegistration::new(path_buf) {
                    Ok(instance) => instance,
                    Err(error) => match error {
                        RegistrationError::CompilationError(analysis_error) => {
                            return Err(load_on_error(analysis_error))
                        }
                        RegistrationError::DynlinkError(dynlink_error) => {
                            return Err(load_on_error(dynlink_error))
                        }
                    },
                },
            ))
        },
    )
}
