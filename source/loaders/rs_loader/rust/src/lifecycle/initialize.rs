use crate::{c_void, LoaderLifecycleState};

#[no_mangle]
pub extern "C" fn rs_loader_impl_initialize(
    _loader_impl: *mut c_void,
    _config: *mut c_void,
) -> *mut c_void {
    let boxed_loader_lifecycle_state = Box::new(LoaderLifecycleState::new(Vec::new()));

    Box::into_raw(boxed_loader_lifecycle_state) as *mut c_void
}
