use crate::{ c_void, c_int };

#[no_mangle]
pub extern "C" fn rs_loader_impl_discover(_loader_impl: *mut c_void, _handle: *mut c_void, _ctx: *mut c_void) -> c_int {
	0 as c_int
}
