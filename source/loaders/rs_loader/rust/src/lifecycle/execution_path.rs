use compiler::api;
use std::ffi::CStr;
use std::os::raw::{c_char, c_int, c_void};
use std::path::PathBuf;

/// # Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_execution_path(
    loader_impl: *mut c_void,
    path: *const c_char,
) -> c_int {
    let result = std::panic::catch_unwind(|| {
        if loader_impl.is_null() || path.is_null() {
            return 1_i32; 
        }

        let loader_lifecycle_state = match api::get_loader_lifecycle_state(loader_impl).as_mut() {
            Some(state) => state,
            None => return 1_i32,
        };

        let path_slice = match CStr::from_ptr(path).to_str() {
            Ok(s) => s,
            Err(_) => return 1_i32,
        };

        loader_lifecycle_state
            .execution_paths
            .push(PathBuf::from(path_slice));

        0_i32 
    });

    result.unwrap_or(1_i32)
}