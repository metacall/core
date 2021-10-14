use crate::{bridge_api, c_char, c_int, c_void, CStr, PathBuf};
use parser::{self, Source};

use std::fs;

#[no_mangle]
pub extern "C" fn rs_loader_impl_execution_path(
    loader_impl: *mut c_void,
    path: *const c_char,
) -> c_int {
    let loader_lifecycle_state = bridge_api::get_loader_lifecycle_state(loader_impl);

    let c_path: &CStr = unsafe { CStr::from_ptr(path) };

    let path_slice: &str = c_path.to_str().unwrap();

    unsafe {
        (*(loader_lifecycle_state))
            .paths
            .push(PathBuf::from(path_slice));
    }

    parser::parse_functions(Source::new(
        fs::read_to_string("/home/raymond/projects/side-projects/metacall/core/build/test.rs")
            .unwrap(),
        String::from("test.rs"),
    ));

    0 as c_int
}
