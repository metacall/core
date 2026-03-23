use super::loader::LoadingMethod;
use std::os::raw::{c_int, c_void};
use compiler::api;

///# Safety
#[no_mangle]
pub unsafe extern "C" fn rs_loader_impl_clear(loader_impl: *mut c_void, handle: *mut c_void) -> c_int {
    let result = std::panic::catch_unwind(|| {

		if loader_impl.is_null() || handle.is_null() {
            return 1_i32;
        }
		
        let loader_lifecycle_state = match api::get_loader_lifecycle_state(loader_impl).as_mut() {
            Some(state) => state,
            None => return 1_i32,
        };

        let methods = Box::from_raw(handle as *mut Vec<LoadingMethod>);

        for loading_method in *methods {
            match loading_method.consume_dlib() {
                Ok(lib) => {
                    loader_lifecycle_state.destroy_list.push(lib);
                }
                Err(err) => {
                    eprintln!("Error consuming dlib: {}", err);
                    return 1_i32;
                }
            }
        }
        
        0_i32
    });

	result.unwrap_or(1_i32)
}
