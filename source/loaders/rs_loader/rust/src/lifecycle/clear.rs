use super::loader::LoadingMethod;
use std::os::raw::{c_int, c_void};

use compiler::api;

#[no_mangle]
pub extern "C" fn rs_loader_impl_clear(loader_impl: *mut c_void, handle: *mut c_void) -> c_int {
    let loader_lifecycle_state = unsafe {
        api::get_loader_lifecycle_state(loader_impl)
            .as_mut()
            .expect("Unable to get loader state")
    };
    let methods = unsafe { Box::from_raw(handle as *mut Vec<LoadingMethod>) };
    for loading_method in *methods {
        match loading_method.consume_dlib() {
            Ok(lib) => {
                // extend the lifetime of library
                loader_lifecycle_state.destroy_list.push(lib);
            }
            Err(err) => {
                eprintln!("{}", err);
                return 1 as c_int;
            }
        }
    }
    0 as c_int
}
