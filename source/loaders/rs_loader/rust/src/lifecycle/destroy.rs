use crate::{c_int, c_void, LoaderLifecycleState};

extern "C" {
    fn loader_impl_get(loader_impl: *mut c_void) -> *mut c_void;
}

#[no_mangle]
pub extern "C" fn rs_loader_impl_destroy(loader_impl: *mut c_void) -> c_int {
    let loader_impl = unsafe { loader_impl_get(loader_impl) };

    let loader_impl = loader_impl as *mut LoaderLifecycleState;

    unsafe {
        loader_impl.drop_in_place();
    }

    0 as c_int
}
