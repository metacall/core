use crate::{c_char, c_int, c_void};

#[no_mangle]
pub extern "C" fn rs_loader_impl_load_from_memory(
    _loader_impl: *mut c_void,
    _name: *const c_char,
    _buffer: *const c_char,
    _size: usize,
) -> *mut c_void {
    1 as c_int as *mut c_void
}
