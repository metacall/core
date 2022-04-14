use crate::{c_int, c_void};
use super::loader::LoadingMethod;

#[no_mangle]
pub extern "C" fn rs_loader_impl_clear(_loader_impl: *mut c_void, handle: *mut c_void) -> c_int {
    let handle_shared_objects = unsafe { Box::from_raw(handle as *mut Vec<LoadingMethod>) };
    drop(handle_shared_objects);
    0 as c_int
}
