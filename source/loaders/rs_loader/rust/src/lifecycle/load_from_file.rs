use super::clear::rs_loader_impl_clear;
use super::loader::{self, LoadingMethod};
use super::template::instantiations_from_data;
use std::cell::RefCell;
use std::os::raw::{c_char, c_void};
use std::path::PathBuf;

use compiler::{file::FileRegistration, RegistrationError};

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_load_from_file_path(
    loader_impl: *mut c_void,
    paths: *const c_char,
    path_size: usize,
    size: usize,
    data: *mut c_void,
) -> *mut c_void {
    let instantiations = match instantiations_from_data(data) {
        Ok(instantiations) => instantiations,
        Err(error) => return loader::load_on_error(error),
    };

    // Number of files which declare the template of each instantiation
    let found = RefCell::new(vec![0_usize; instantiations.len()]);

    let handle = loader::load(
        loader_impl,
        paths,
        path_size,
        size,
        |path_buf: PathBuf,
         load_on_error: loader::LoadOnErrorPointer|
         -> Result<LoadingMethod, *mut c_void> {
            let instance = match FileRegistration::new(path_buf, &instantiations) {
                Ok(instance) => instance,
                Err(error) => match error {
                    RegistrationError::CompilationError(analysis_error) => {
                        return Err(load_on_error(analysis_error))
                    }
                    RegistrationError::DynlinkError(dynlink_error) => {
                        return Err(load_on_error(dynlink_error))
                    }
                },
            };

            for (count, instantiation) in found.borrow_mut().iter_mut().zip(&instantiations) {
                if instance.state.instantiations().contains(instantiation) {
                    *count += 1;
                }
            }

            Ok(LoadingMethod::File(instance))
        },
    );

    if handle.is_null() {
        return handle;
    }

    // Each template must be declared in exactly one of the files
    let error = instantiations
        .iter()
        .zip(found.into_inner())
        .find_map(|(instantiation, count)| match count {
            0 => Some(format!("Template '{}' not found.", instantiation.template)),
            1 => None,
            _ => Some(format!(
                "Template '{}' found in more than one file.",
                instantiation.template
            )),
        });

    match error {
        Some(error) => {
            unsafe { rs_loader_impl_clear(loader_impl, handle) };
            loader::load_on_error(error)
        }
        None => handle,
    }
}
