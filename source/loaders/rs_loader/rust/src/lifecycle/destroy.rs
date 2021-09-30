use crate::{c_int, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_destroy(_loader_impl: *mut c_void) -> c_int {
    0 as c_int
}
