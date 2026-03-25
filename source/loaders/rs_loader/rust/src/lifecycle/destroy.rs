use std::os::raw::{c_int, c_void};

use compiler::api;

#[no_mangle]
pub extern "C" fn rs_loader_impl_destroy(loader_impl: *mut c_void) -> c_int {
    if loader_impl.is_null() {
        eprintln!("rs_loader_impl_destroy: received null loader_impl pointer");
        return 1_i32;
    }

    let loader_lifecycle_state = api::get_loader_lifecycle_state(loader_impl);

    if loader_lifecycle_state.is_null() {
        eprintln!("rs_loader_impl_destroy: lifecycle state is null");
        return 1_i32;
    }

    // unload children, prevent memory leaks
    api::loader_lifecycle_unload_children(loader_impl);

    // drop the state
    unsafe {
        Box::from_raw(loader_lifecycle_state);
    }

    0_i32
}
