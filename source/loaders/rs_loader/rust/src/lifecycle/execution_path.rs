use compiler::api;
use std::ffi::CStr;
use std::os::raw::{c_char, c_int, c_void};
use std::path::PathBuf;

#[no_mangle]
pub extern "C" fn rs_loader_impl_execution_path(
    loader_impl: *mut c_void,
    path: *const c_char,
) -> c_int {
    let loader_lifecycle_state = unsafe {
        api::get_loader_lifecycle_state(loader_impl)
            .as_mut()
            .expect("Unable to get lifecycle state.")
    };

    let c_path: &CStr = unsafe { CStr::from_ptr(path) };

    let path_slice: &str = c_path.to_str().expect("Unable to cast CStr to str");
    loader_lifecycle_state
        .execution_paths
        .push(PathBuf::from(path_slice));

    0 as c_int
}
