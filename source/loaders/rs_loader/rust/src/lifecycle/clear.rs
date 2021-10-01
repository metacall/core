use crate::{c_int, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_clear(_loader_impl: *mut c_void, _handle: *mut c_void) -> c_int {
    0 as c_int
}
