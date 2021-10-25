use super::bridge_api;
use crate::{c_int, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_destroy(loader_impl: *mut c_void) -> c_int {
    let loader_lifecycle_state = bridge_api::get_loader_lifecycle_state(loader_impl);

    unsafe {
        loader_lifecycle_state.drop_in_place();
    }

    0 as c_int
}
