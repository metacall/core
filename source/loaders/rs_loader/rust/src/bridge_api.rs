use crate::{c_void, LoaderLifecycleState};

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;
}

pub fn get_loader_lifecycle_state(loader_impl: *mut c_void) -> *mut LoaderLifecycleState {
    let loader_lifecycle_state =
        unsafe { loader_impl_get(loader_impl) } as *mut LoaderLifecycleState;

    loader_lifecycle_state
}
