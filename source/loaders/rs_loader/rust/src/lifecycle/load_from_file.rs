use super::loader::{self, LoadingMethod};
use std::os::raw::{c_char, c_void};
use std::path::PathBuf;

use compiler::{file::FileRegistration, RegistrationError};

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_file(
    loader_impl: *mut c_void,
    paths: *mut *const c_char,
    size: usize,
) -> *mut c_void {
    loader::load(
        loader_impl,
        paths,
        size,
        true,
        |path_buf: PathBuf,
         load_on_error: loader::LoadOnErrorPointer|
         -> Result<LoadingMethod, *mut c_void> {
            Ok(LoadingMethod::File(match FileRegistration::new(path_buf) {
                Ok(instance) => instance,
                Err(error) => match error {
                    RegistrationError::CompilationError(analysis_error) => {
                        return Err(load_on_error(analysis_error))
                    }
                    RegistrationError::DynlinkError(dynlink_error) => {
                        return Err(load_on_error(dynlink_error))
                    }
                },
            }))
        },
    )
}
