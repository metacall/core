use compiler::api;
use std::ffi::CStr;
use std::os::raw::{c_char, c_int, c_void};
use std::path::PathBuf;

/// # Safety
///
/// This function is called from C code. Both `loader_impl` and `path` must be
/// valid, non-null pointers. `path` must point to a null-terminated C string
/// containing valid UTF-8.
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_execution_path(
    loader_impl: *mut c_void,
    path: *const c_char,
) -> c_int {
    if loader_impl.is_null() || path.is_null() {
        eprintln!("rs_loader_impl_execution_path: received null pointer");
        return 1_i32;
    }

    let loader_lifecycle_state = match api::get_loader_lifecycle_state(loader_impl).as_mut() {
        Some(state) => state,
        None => {
            eprintln!("rs_loader_impl_execution_path: unable to get lifecycle state");
            return 1_i32;
        }
    };

    let c_path: &CStr = CStr::from_ptr(path);

    let path_slice: &str = match c_path.to_str() {
        Ok(s) => s,
        Err(_) => {
            eprintln!("rs_loader_impl_execution_path: path is not valid UTF-8");
            return 1_i32;
        }
    };
    loader_lifecycle_state
        .execution_paths
        .push(PathBuf::from(path_slice));

    0_i32
}
