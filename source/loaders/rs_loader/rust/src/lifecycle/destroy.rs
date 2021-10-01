use crate::{c_int, c_void, LoaderLifecycleState};

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;
}

#[no_mangle]
pub extern "C" fn rs_loader_impl_destroy(loader_impl: *mut c_void) -> c_int {
    let loader_lifecycle_state =
        unsafe { loader_impl_get(loader_impl) } as *mut LoaderLifecycleState;

    unsafe {
        loader_lifecycle_state.drop_in_place();
    }

    0 as c_int
}
