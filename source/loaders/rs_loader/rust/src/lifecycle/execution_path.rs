use std::path::Path;

use crate::{c_char, c_int, c_void, CStr, LoaderLifecycleState, PathBuf};

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;
}

#[no_mangle]
pub extern "C" fn rs_loader_impl_execution_path(
    loader_impl: *mut c_void,
    path: *const c_char,
) -> c_int {
    let loader_lifecycle_state =
        unsafe { loader_impl_get(loader_impl) } as *mut LoaderLifecycleState;

    let c_path: &CStr = unsafe { CStr::from_ptr(path) };

    let path_slice: &str = c_path.to_str().unwrap();

    unsafe {
        (*(loader_lifecycle_state))
            .paths
            .push(PathBuf::from(path_slice));
    }

    0 as c_int
}
