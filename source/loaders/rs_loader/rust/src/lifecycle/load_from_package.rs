use crate::{c_char, c_void};

use super::loader::{self, LoadingMethod};
use crate::PathBuf;
use compiler::{package::PackageRegistration, RegistrationError};
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
                        RegistrationError::DlopenError(dlopen_error) => {
                            return Err(load_on_error(dlopen_error))
                        }
                    },
                },
            ))
        },
    )
}
