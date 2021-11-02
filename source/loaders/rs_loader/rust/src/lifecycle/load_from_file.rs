use super::loader::{self, LoadingMethod};
use crate::{c_char, c_void, PathBuf};

use metacall_registrator::{file::FileRegistration, RegistrationError};

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
                    RegistrationError::SynError(syn_error) => {
                        return Err(load_on_error(syn_error.to_string()))
                    }
                    RegistrationError::ValidationError(validation_error) => {
                        return Err(load_on_error(validation_error))
                    }
                    RegistrationError::DlopenError(dlopen_error) => {
                        return Err(load_on_error(dlopen_error))
                    }
                },
            }))
        },
    )
}
